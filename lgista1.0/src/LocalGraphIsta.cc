// SPDX-License-Identifier: BSD-3-Clause
//
// LocalGraphIsta facade implementation. See LocalGraphIsta.hh.

#include "lgista/LocalGraphIsta.hh"

#include <cstdio>

#include "db_sta/dbSta.hh"
#include "sta/Network.hh"
#include "sta/Sta.hh"

#include "LocalSta.hh"
#include "PtGraph.hh"
#include "LocalDmpDelayCalc.hh"
#include "IstaWriteback.hh"

namespace lgista {

LocalGraphIsta::LocalGraphIsta(sta::dbSta* sta) : sta_(sta)
{
  // Construction is deferred to prepare(): LocalSta::copyState and the local
  // delay calculator need a built/levelized global timing graph, which only
  // exists after updateTiming(). Building them here (before any timing run)
  // dereferences a not-yet-built graph and faults.
}

LocalGraphIsta::~LocalGraphIsta()
{
  delete arc_delay_calc_;
  delete local_sta_;
}

void LocalGraphIsta::prepare()
{
  if (prepared_)
    return;
  // Bring the global timing graph fully up to date (ensures graph build +
  // levelization + arrivals + requireds) so the local graph can read upstream
  // slews / arrivals from it. updateTiming(true) is robust even when the
  // design has no clocks/constraints (findRequireds alone can fault then).
  sta_->updateTiming(/*full=*/true);
  local_sta_ = new LocalSta(sta_);
  local_sta_->copyState(sta_);
  arc_delay_calc_ = lgista::makeLocalDelayCalc(sta_);
  prepared_ = true;
}

PtGraph* LocalGraphIsta::analyzeInstance(sta::Instance* inst, bool print_report)
{
  if (inst == nullptr)
    return nullptr;
  if (!prepared_)
    prepare();

  PtGraph* pt_graph = local_sta_->makePtGraph(inst, /*update_timing_first=*/true);
  if (pt_graph == nullptr)
    return nullptr;

  local_sta_->findLocalDelays(pt_graph, arc_delay_calc_);
  local_sta_->findLocalArrivals(pt_graph);
  local_sta_->findLocalRequireds(pt_graph);

  if (print_report)
    local_sta_->printLocalTiming(pt_graph);

  return pt_graph;
}

PtGraph* LocalGraphIsta::analyzeInstance(const std::string& inst_name,
                                         bool print_report)
{
  sta::Network* network = sta_->network();
  sta::Instance* inst = network->findInstance(inst_name.c_str());
  if (inst == nullptr) {
    std::printf("LocalGraphIsta: instance '%s' not found\n", inst_name.c_str());
    return nullptr;
  }
  return analyzeInstance(inst, print_report);
}

void LocalGraphIsta::commitToGlobal(PtGraph* pt_graph)
{
  if (pt_graph != nullptr)
    updateTimingFromPtGraph(pt_graph);
}

void LocalGraphIsta::ensureInit()
{
  if (prepared_)
    return;
  local_sta_ = new LocalSta(sta_);
  local_sta_->copyState(sta_);
  arc_delay_calc_ = lgista::makeLocalDelayCalc(sta_);
  prepared_ = true;
}

bool LocalGraphIsta::analyzeAndCommit(sta::Instance* inst)
{
  if (inst == nullptr)
    return false;
  ensureInit();

  PtGraph pt_graph(sta_);
  if (!local_sta_->makePtGraph(&pt_graph, inst))
    return false;

  local_sta_->findLocalDelays(&pt_graph, arc_delay_calc_);
  local_sta_->findLocalArrivals(&pt_graph);
  local_sta_->findLocalRequireds(&pt_graph);

  updateTimingFromPtGraph(&pt_graph);
  return true;
}

size_t LocalGraphIsta::analyzeAll(bool print_report)
{
  if (!prepared_)
    prepare();

  sta::Network* network = sta_->network();
  size_t built = 0;
  sta::LeafInstanceIterator* iter = network->leafInstanceIterator();
  while (iter->hasNext()) {
    sta::Instance* inst = iter->next();
    if (analyzeInstance(inst, print_report) != nullptr)
      ++built;
  }
  delete iter;
  return built;
}

}  // namespace lgista
