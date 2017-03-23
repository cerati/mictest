#ifndef CandCloner_h
#define CandCloner_h

#include "MkFitter.h"

#include <vector>

//#define CC_TIME_LAYER
//#define CC_TIME_ETA

typedef std::pair<int, int> CandClonerWork_t;

class CandCloner
{
public:
  // Maximum number of seeds processed in one call to ProcessSeedRange()
  static const int s_max_seed_range = MPT_SIZE;

private:
  // Temporaries in ProcessSeedRange(), re-sized/served  in constructor.

  // Size of this one is s_max_seed_range
  std::vector<std::vector<Track> > t_cands_for_next_lay;

public:
  CandCloner()
  {
    // m_fitter = new (_mm_malloc(sizeof(MkFitter), 64)) MkFitter(0);

    t_cands_for_next_lay.resize(s_max_seed_range);
    for (int iseed = 0; iseed < s_max_seed_range; ++iseed)
    {
      t_cands_for_next_lay[iseed].reserve(Config::maxCandsPerSeed);
    }
  }

  ~CandCloner()
  {
  }

  void begin_eta_bin(EtaRegionOfCombCandidates * er_o_ccs, int start_seed, int n_seeds)
  {
    mp_region_of_comb_candidates = er_o_ccs;
    m_start_seed = start_seed;
    m_n_seeds    = n_seeds;
    m_hits_to_add.resize(n_seeds);

    // XXX Should resize vectors in m_hits_to_add to whatever makes sense ???
    // for (int i = 0; i < n_seeds; ++i)
    // {
    //   m_hits_to_add[i].reserve(20 * Config::maxCandsPerSeed);
    // }

#ifdef CC_TIME_ETA
    printf("CandCloner::begin_eta_bin\n");
    t_eta = dtime();
#endif
  }

  void begin_layer(int lay)
  {
    m_layer = lay;

    m_idx_max      = 0;
    m_idx_max_prev = 0;

#ifdef CC_TIME_LAYER
    t_lay = dtime();
#endif
  }

  void begin_iteration()
  {
    // Do nothing, "secondary" state vars updated when work completed/assigned.
  }

  void add_cand(int idx, const MkFitter::IdxChi2List& cand_info)
  {
    m_hits_to_add[idx].push_back(cand_info);

    m_idx_max = std::max(m_idx_max, idx);
  }

  int num_cands(int idx)
  {
    return m_hits_to_add[idx].size();
  }

  void end_iteration()
  {
    int proc_n = m_idx_max - m_idx_max_prev;

    // printf("CandCloner::end_iteration process %d, max_prev=%d, max=%d\n", proc_n, m_idx_max_prev, m_idx_max);

    if (proc_n >= s_max_seed_range)
    {
      // Round to multiple of s_max_seed_range.
      DoWork((m_idx_max / s_max_seed_range) * s_max_seed_range);
    }
  }

  void end_layer()
  {
    if (m_n_seeds > m_idx_max_prev)
    {
      DoWork(m_n_seeds);
    }

    for (int i = 0; i < m_n_seeds; ++i)
    {
      m_hits_to_add[i].clear();
    }

#ifdef CC_TIME_LAYER
    t_lay = dtime() - t_lay;
    printf("CandCloner::end_layer %d -- t_lay=%8.6f\n", m_layer, t_lay);
    printf("                      m_idx_max=%d, m_idx_max_prev=%d, issued work=%d\n",
           m_idx_max, m_idx_max_prev, m_idx_max + 1 > m_idx_max_prev);
#endif
  }

  void end_eta_bin()
  {
#ifdef CC_TIME_ETA
    t_eta = dtime() - t_eta;
    printf("CandCloner::end_eta_bin t_eta=%8.6f\n", t_eta);
#endif
  }

  void DoWork(int idx)
  {
    // printf("CandCloner::DoWork assigning work from seed %d to %d\n", m_idx_max_prev, idx);

    int beg     = m_idx_max_prev;
    int the_end = idx;

    // printf("CandCloner::DoWork working on beg=%d to the_end=%d\n", beg, the_end);

    while (beg != the_end)
    {
      int end = std::min(beg + s_max_seed_range, the_end);

      // printf("CandCloner::DoWork processing %4d -> %4d\n", beg, end);

      ProcessSeedRange(beg, end);

      beg = end;
    }

    m_idx_max_prev = idx;
  }

  // ----------------------------------------------------------------

  void ProcessSeedRange(int is_beg, int is_end);

  // ----------------------------------------------------------------

  // eventually, protected or private

  int  m_idx_max, m_idx_max_prev;
  std::vector<std::vector<MkFitter::IdxChi2List>> m_hits_to_add;

  EtaRegionOfCombCandidates *mp_region_of_comb_candidates;

#if defined(CC_TIME_ETA) or defined(CC_TIME_LAYER)
  double    t_eta, t_lay;
#endif

  int       m_start_seed, m_n_seeds;
  int       m_layer;
};

#endif
