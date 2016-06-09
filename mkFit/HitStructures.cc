#include "HitStructures.h"
#include "BinInfoUtils.h"

#include "Ice/IceRevisitedRadix.h"

void LayerOfHits::SetupLayer(float zmin, float zmax, float dz)
{
  // XXXX MT: Could have n_phi bins per layer, too.

  assert (m_nz == 0 && "SetupLayer() already called.");

  float nmin = std::floor(zmin / dz);
  float nmax = std::ceil (zmax / dz);
  m_zmin = dz * nmin;
  m_zmax = dz * nmax;
  m_fz = 1.0f / dz; // zbin = (zhit - m_zmin) * m_fz;
  m_nz   = nmax - nmin;

  m_phi_bin_infos.resize(m_nz);
  for (int i = 0; i < m_nz; ++i) m_phi_bin_infos[i].resize(m_nphi);
}

void LayerOfHits::SuckInHits(const HitVec &hitv)
{
  // This is now in SetupLayer()
  // // should be layer dependant
  // float dz = 0.5;
  // // should know this from geom.
  // //m_zmin =  1000;
  // //m_zmax = -1000;
  // for (auto const &h : hitv)
  // {
  //   if (h.z() < m_zmin) m_zmin = h.z();
  //   if (h.z() > m_zmax) m_zmax = h.z();
  // }
  // printf("LoH::SuckInHits zmin=%f, zmax=%f", m_zmin, m_zmax);
  // float nmin = std::floor(m_zmin / dz);
  // float nmax = std::ceil (m_zmax / dz);
  // m_zmin = dz * nmin;
  // m_zmax = dz * nmax;
  // int nz  = nmax - nmin;
  // int nzh = nz / 2;
  // m_fz = 1.0f / dz; // zbin = (zhit - m_zmin) * m_fz;
  // printf(" -> zmin=%f, zmax=%f, nz=%d, fz=%f\n", m_zmin, m_zmax, nz, m_fz);

  assert (m_nz > 0 && "SetupLayer() was not called.");

  const int size = hitv.size();

  if (m_capacity < size)
  {
    free_hits();
    alloc_hits(1.02 * size);
  }

#ifndef LOH_USE_PHI_Z_ARRAYS
  std::vector<float>        m_hit_phis(size);
#endif

  struct HitInfo
  {
    float phi;
    float z;
    int   zbin;
  };

  std::vector<HitInfo> ha(size);
  std::vector<int>     zc(m_nz, 0);
  int nzh = m_nz / 2;
  {
    int i = 0;
    for (auto const &h : hitv)
    {
      HitInfo &hi = ha[i];
      hi.phi  = h.phi();
      hi.z    = h.z();
      hi.zbin = (hi.z - m_zmin) * m_fz;
      m_hit_phis[i] = hi.phi + 6.3f * (hi.zbin - nzh);
      ++zc[hi.zbin];
      ++i;
    }
  }

  RadixSort sort;
  sort.Sort(&m_hit_phis[0], size);

  int curr_z_bin   = 0;
  int curr_phi_bin = 0;
  int hits_in_bin  = 0;
  int hit_count    = 0;

  for (int i = 0; i < size; ++i)
  {
    int j = sort.GetRanks()[i];

    // Could fix the mis-sorts. Set ha size to size + 1 and fake last entry to avoid ifs.

    memcpy(&m_hits[i], &hitv[j], sizeof(Hit));
#ifdef LOH_USE_PHI_Z_ARRAYS
    m_hit_phis[i] = ha[j].phi;
    m_hit_zs  [i] = ha[j].z;
#endif

    // Fill the bin info

    if (ha[j].zbin != curr_z_bin)
    {
      set_phi_bin(curr_z_bin, curr_phi_bin, hit_count, hits_in_bin);

      empty_phi_bins(curr_z_bin, curr_phi_bin + 1, m_nphi, hit_count);

      empty_z_bins(curr_z_bin + 1, ha[j].zbin, hit_count);

      curr_z_bin = ha[j].zbin;
      curr_phi_bin = 0;
    }

    int phi_bin = GetPhiBin(ha[j].phi);

    if (phi_bin > curr_phi_bin)
    {
      set_phi_bin(curr_z_bin, curr_phi_bin, hit_count, hits_in_bin);

      empty_phi_bins(curr_z_bin, curr_phi_bin + 1, phi_bin, hit_count);

      curr_phi_bin = phi_bin;
    }

    ++hits_in_bin;
  }

  set_phi_bin(curr_z_bin, curr_phi_bin, hit_count, hits_in_bin);

  empty_phi_bins(curr_z_bin, curr_phi_bin + 1, m_nphi, hit_count);

  empty_z_bins(curr_z_bin + 1, m_nz, hit_count);

  // Check for mis-sorts due to lost precision (not really important).
  // float phi_prev = 0;
  // int   bin_prev = -1;
  // int   prev_err_idx = -1;
  // for (int i = 0; i < size; ++i)
  // {
  //   int j = sort.GetRanks()[i];
  //   float phi  = ha[j].phi;
  //   int   zbin = ha[j].zbin;
  //   if (zbin == bin_prev && phi < phi_prev)
  //   {
  //     //printf("  Offset error: %5d %5d %10f %10f %5d %8f\n", i, j, phi, phi_prev, ha[j].zbin, hitv[j].z());
  //     if (prev_err_idx == i - 1)
  //       printf("DOUBLE Offset error: %5d %5d %10f %10f %5d %8f\n", i, j, phi, phi_prev, ha[j].zbin, hitv[j].z());
  //     prev_err_idx = i;
  //   }
  //   phi_prev = phi;
  //   bin_prev = zbin;
  // }

  // Print first couple
  // for(int i = 0; i < 20; ++i)
  // {
  //   int j = sort.GetRanks()[i];
  //
  //   printf("%3d %3d %8f %5d %8f\n", i, j, ha[j].phi, ha[j].zbin, hitv[j].z());
  // }
}

int LayerOfHits::SelectHitIndices(float z, float phi, float dz, float dphi, bool dump)
{
  // Sanitizes z, dz and dphi. phi is expected to be in -pi, pi.

  // Make sure how phi bins work beyond -pi, +pi.
  // for (float p = -8; p <= 8; p += 0.05)
  // {
  //   int pb = GetPhiBin(p);
  //   printf("%5.2f %4d %4d\n", p, pb, pb & m_phi_mask);
  // }

  if (std::abs(dz)   > m_max_dz)   dz   = m_max_dz;
  if (std::abs(dphi) > m_max_dphi) dphi = m_max_dphi;

  int zb1 = GetZBinChecked(z - dz);
  int zb2 = GetZBinChecked(z + dz) + 1;
  int pb1 = GetPhiBin(phi - dphi);
  int pb2 = GetPhiBin(phi + dphi) + 1;

  // int extra = 2;
  // zb1 -= 2; if (zb < 0) zb = 0;
  // zb2 += 2; if (zb >= m_nz) zb = m_nz;

  if (dump)
    printf("LayerOfHits::SelectHitIndices %6.3f %6.3f %6.4f %7.5f %3d %3d %4d %4d\n",
           z, phi, dz, dphi, zb1, zb2, pb1, pb2);

  // This should be input argument, well ... it will be Matriplex op, or sth.
  std::vector<int> idcs;

  for (int zi = zb1; zi < zb2; ++zi)
  {
    for (int pi = pb1; pi < pb2; ++pi)
    {
      int pb = pi & m_phi_mask;

      for (int hi = m_phi_bin_infos[zi][pb].first; hi < m_phi_bin_infos[zi][pb].second; ++hi)
      {
        // Here could enforce some furhter selection on hits
#ifdef LOH_USE_PHI_Z_ARRAYS
        float ddz   = std::abs(z   - m_hit_zs[hi]);
        float ddphi = std::abs(phi - m_hit_phis[hi]);
        if (ddphi > Config::PI) ddphi = Config::TwoPI - ddphi;

        if (dump)
          printf("     SHI %3d %4d %4d %5d  %6.3f %6.3f %6.4f %7.5f   %s\n",
                 zi, pi, pb, hi,
                 m_hit_zs[hi], m_hit_phis[hi], ddz, ddphi,
                 (ddz < dz && ddphi < dphi) ? "PASS" : "FAIL");

        if (ddz < dz && ddphi < dphi)
#endif
        {
          idcs.push_back(hi);
        }
      }
    }
  }

  return idcs.size();
}

void LayerOfHits::PrintBins()
{
  for (int zb = 0; zb < m_nz; ++zb)
  {
    printf("Z bin %d\n", zb);
    for (int pb = 0; pb < m_nphi; ++pb)
    {
      if (pb % 8 == 0)
        printf(" Phi %4d: ", pb);
      printf("%5d,%4d   %s",
             m_phi_bin_infos[zb][pb].first, m_phi_bin_infos[zb][pb].second,
             ((pb + 1) % 8 == 0) ? "\n" : "");
    }
  }
}
