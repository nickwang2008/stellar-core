#pragma once

// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "util/Timer.h"
#include "Application.h"
#include "main/Config.h"
#include "main/PersistentState.h"
#include <thread>

namespace stellar
{
class TmpDirManager;
class LedgerManager;
class Herder;
class CLFManager;
class HistoryManager;
class ProcessManager;
class CommandHandler;
class Database;

class ApplicationImpl : public Application
{
  public:
    ApplicationImpl(VirtualClock& clock, Config const& cfg);
    virtual ~ApplicationImpl() override;

    virtual uint64_t timeNow() override;

    virtual Config const& getConfig() override;

    virtual State getState() override;
    virtual void setState(State) override;
    virtual VirtualClock& getClock() override;
    virtual medida::MetricsRegistry& getMetrics() override;
    virtual TmpDirManager& getTmpDirManager() override;
    virtual LedgerManager& getLedgerManager() override;
    virtual CLFManager& getCLFManager() override;
    virtual HistoryManager& getHistoryManager() override;
    virtual ProcessManager& getProcessManager() override;
    virtual Herder& getHerder() override;
    virtual OverlayManager& getOverlayManager() override;
    virtual Database& getDatabase() override;
    virtual PersistentState& getPersistentState() override;

    virtual asio::io_service& getWorkerIOService() override;

    virtual void start() override;

    // Stops the io_services, which should cause the threads to exit
    // once they finish running any work-in-progress. If you want a
    // more abrupt exit than this, call exit() and hope for the best.
    virtual void gracefulStop() override;

    // Wait-on and join all the threads this application started; should
    // only return when there is no more work to do or someone has
    // force-stopped the io_services. Application can be safely destroyed
    // after this returns.
    virtual void joinAllThreads() override;

    virtual bool manualClose() override;

    virtual void applyCfgCommands() override;

    virtual void reportCfgMetrics() override;

  private:
    Application::State mState;
    VirtualClock& mVirtualClock;
    Config mConfig;

    // NB: The io_services should come first, then the 'manager'
    // sub-objects, then the threads. Do not reorder these fields.
    //
    // The fields must be constructed in this order, because the
    // 'manager' sub-objects register work-to-do (listening on sockets)
    // with the io_services during construction, and the threads are
    // activated immediately thereafter to serve requests; if the
    // threads started first, they would try to do work, find no work,
    // and exit.
    //
    // The fields must be destructed in the reverse order because the
    // 'manager' sub-objects contain various IO objects that refer
    // directly to the io_services.

    asio::io_service mWorkerIOService;
    std::unique_ptr<asio::io_service::work> mWork;

    std::unique_ptr<medida::MetricsRegistry> mMetrics;
    std::unique_ptr<Database> mDatabase;
    std::unique_ptr<TmpDirManager> mTmpDirManager;
    std::unique_ptr<OverlayManager> mOverlayManager;
    std::unique_ptr<LedgerManager> mLedgerManager;
    std::unique_ptr<Herder> mHerder;
    std::unique_ptr<CLFManager> mCLFManager;
    std::unique_ptr<HistoryManager> mHistoryManager;
    std::unique_ptr<ProcessManager> mProcessManager;
    std::unique_ptr<CommandHandler> mCommandHandler;
    std::unique_ptr<PersistentState> mPersistentState;

    std::vector<std::thread> mWorkerThreads;

    asio::signal_set mStopSignals;

    void runWorkerThread(unsigned i);
};
}