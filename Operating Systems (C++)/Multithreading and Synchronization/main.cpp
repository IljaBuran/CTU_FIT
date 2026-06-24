#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <compare>
#include <condition_variable>
#include <span>
#include <optional>
#include "progtest_solver.h"
#include "sample_tester.h"
#include <stdint.h>
#endif /* __PROGTEST__ */

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using msgId = u32;

namespace helpers
{
    static msgId get_message_id(u64 msg)
    {
        return (u32)(msg >> SHIFT_MSG_ID);
    }

    static u8 get_fragment_count(u64 msg)
    {
        return (u8)((msg >> SHIFT_FRAGMENT_CNT) & MASK_FRAGMENT_CNT);
    }

    static bool _is_msg_valid(msgId id, const std::vector<u64>& fragments)
    {
        if (fragments.empty())
            return false;

        const u8 fragCnt = helpers::get_fragment_count(fragments.front());
        const size_t expected = (fragCnt + 1);

        if (fragments.size() != expected)
            return false;

        for (u64 x : fragments)
            if (helpers::get_message_id(x) != id || helpers::get_fragment_count(x) != fragCnt)
                return false;

        return true;
    }
}

class CSentinelHacker
{
private:
    
    struct MessageState
    {
        std::vector<u64>       fragments;
        u8                     fragCnt      = std::numeric_limits<u8>::max();
        bool                   finished     = false;
        bool                   sent         = false;
        bool                   invalid      = false;
        std::optional<CBigInt> result       = std::nullopt;

        inline bool _is_fragCnt_known() const
        {
            return fragCnt != std::numeric_limits<u8>::max();
        }

        inline bool _is_submitted() const
        {
            return (fragments.size() >= (u64)fragCnt + 1);
        }
    };

    class SerializerWrapper
    {
    private:
        
        std::mutex     mtxSer;
        std::mutex&    refMtxWork;
        
        std::condition_variable& refCvWork;
        
        AMsgSerializer pCurrent;

        std::queue<std::function<void()>>& refQTask;

    public:

        SerializerWrapper(std::queue<std::function<void()>>& qTask,
                          std::mutex&                        mtxWork,
                          std::condition_variable&           cvWork)
            : refMtxWork(mtxWork), refCvWork(cvWork), refQTask(qTask) {}

        void add_problem(std::vector<u64>                                       fragments,
                         std::function<void(uint32_t, const uint8_t[], size_t)> foundFn,
                         std::function<void(uint32_t)>                          finishedFn)
        {
            AMsgSerializer ser;
            
            {
                std::lock_guard<std::mutex> lg(mtxSer);

                if (!pCurrent)
                    pCurrent = createMsgSerializer();

                bool added = pCurrent->addProblem(std::move(fragments), std::move(foundFn), std::move(finishedFn));
                assert(added); // dont fail or its gg

                if (!pCurrent->hasFreeCapacity())
                    ser = std::move(pCurrent);
            }
            
            if (ser)
                _schedule(std::move(ser));
        }

        void force_flush()
        {
            AMsgSerializer ser;
            {
                std::lock_guard<std::mutex> lg(mtxSer);
                if (pCurrent)
                    ser = std::move(pCurrent);
            }
            if (ser)
                _schedule(std::move(ser));
        }

    private:

        void _schedule(AMsgSerializer ser)
        {
            const u32 threadsNeeded = ser->totalThreads();
            {
                std::lock_guard<std::mutex> lg(refMtxWork);
                for (u32 i = 0; i < threadsNeeded; i++)
                {
                    refQTask.emplace(
                        [ser]
                        {
                            ser->solve();
                        });
                }
            }
            refCvWork.notify_all();
        }
    };

    struct TransmitInfo
    {
        msgId                  id;
        std::optional<CBigInt> value;
    };

    std::vector<AReceiver>    receivers;
    std::vector<ATransmitter> transmitters;
    
    std::vector<std::thread> receivingThreads;
    std::vector<std::thread> workingThreads;
    std::vector<std::thread> transmittingThreads;
    
    std::unordered_map<msgId, MessageState> messages;
    
    std::queue<std::function<void()>> qTasks;
    std::queue<TransmitInfo>          qOut;
    
    std::mutex mtxMsg;
    std::mutex mtxOut;
    std::mutex mtxWork;
    std::mutex mtxDone;
    
    std::condition_variable cvWork;
    std::condition_variable cvDone;
    std::condition_variable cvOut;
    
    SerializerWrapper serializer;
    
    std::atomic<u64>  aActiveProblems  = 0;
    std::atomic<u64>  aActiveOut       = 0;
    std::atomic<bool> aWorkerStop      = false;
    std::atomic<bool> aTransmitterStop = false;

public:

    CSentinelHacker()
        : serializer(qTasks, mtxWork, cvWork) {}

    static bool seqSolve(const std::vector<u64>& fragments, CBigInt& res)
    {
        if (fragments.empty())
        return false;
        
        const msgId id = helpers::get_message_id(fragments.front());
        if (!helpers::_is_msg_valid(id, fragments))
            return false;

        AMsgSerializer serializer = createMsgSerializer();
        if (!serializer)
            return false;

        std::mutex m;
        CBigInt best     = 0;
        bool    found    = false;
        bool    finished = false;
        u32     cnt      = 0;

        auto foundFn = [&](msgId, const u8 data[], size_t bits)
        {
            CBigInt curr = countExpressions(data, bits);
            std::lock_guard<std::mutex> lg(m);
            if (!found || best < curr)
                best = curr;
            found = true;
        };

        auto finishedFn = [&](u32 x)
        {
            std::lock_guard<std::mutex> lg(m);
            cnt      = x;
            finished = true;
        };

        if (!serializer->addProblem(fragments, foundFn, finishedFn))
            return false;

        const u32 threadsNeeded = serializer->totalThreads();
        std::vector<std::thread> tmp;
        tmp.reserve(threadsNeeded);
        for (u32 i = 0; i < threadsNeeded; i++)
            tmp.emplace_back(
                [serializer]() 
                { 
                    serializer->solve(); 
                });

        for (auto& t : tmp)
            t.join();

        if (!finished || !cnt || !found)
            return false;

        res = best;
        return true;
    }

    void addTransmitter(ATransmitter x)
    {
        transmitters.push_back(std::move(x));
    }

    void addReceiver(AReceiver x)
    {
        receivers.push_back(std::move(x));
    }

    void addFragment(u64 x)
    {
        std::optional<std::vector<u64>> toSubmit = std::nullopt;
        msgId id = helpers::get_message_id(x);
        u8 fragCnt = helpers::get_fragment_count(x);
        
        {
            std::lock_guard<std::mutex> lg(mtxMsg);
            MessageState& msgSt = messages[id];
            
            // if marked as invalid or it has too many fragments -> ignore
            if (msgSt.invalid || msgSt.fragments.size() >= (u64)msgSt.fragCnt + 1) 
                return;

            if (!msgSt._is_fragCnt_known())
                msgSt.fragCnt = fragCnt;
            else if (msgSt.fragCnt != fragCnt)
            {
                msgSt.invalid = true;
                return;
            }
            
            msgSt.fragments.push_back(x);

            if (msgSt.fragments.size() == (u64)msgSt.fragCnt + 1)
                toSubmit = msgSt.fragments;
        }

        if (toSubmit)
            _submit_message_to_solve(id, std::move(*toSubmit));
    }

    void start(u32 thrCount)
    {
        receivingThreads.reserve(receivers.size());
        for (const auto& r : receivers)
            receivingThreads.emplace_back(
                [this, r]() 
                { 
                    _receiver_job(r); 
                });

        workingThreads.reserve(thrCount);
        for (u32 i = 0; i < thrCount; i++)
            workingThreads.emplace_back(
                [this]() 
                { 
                    _worker_job(); 
                });

        transmittingThreads.reserve(transmitters.size());
        for (const auto& t : transmitters)
            transmittingThreads.emplace_back(
                [this, t]() 
                { 
                    _transmitter_job(t); 
                });
    }

    void stop()
    {
        // wait until all the receivers shut down
        for (auto& receiver : receivingThreads)
            receiver.join();
        
        // force all problems in serializer to be solved and wait for them
        serializer.force_flush();
        {
            std::unique_lock<std::mutex> ul(mtxDone);
            cvDone.wait(ul, 
                [this]
                {
                    return aActiveProblems == 0;
                });
        }
        
        // stop workers
        aWorkerStop = true;
        cvWork.notify_all();

        for (auto& worker : workingThreads)
            worker.join();

        // collect all the unsent messages
        std::vector<TransmitInfo> unsent;
        {
            std::lock_guard<std::mutex> lg(mtxMsg);
            for (auto& [id, msg] : messages)
            {
                if (msg.sent)
                    continue;
                msg.sent = true;
                TransmitInfo t{id, msg.result};
                unsent.push_back(std::move(t));
            }
        }

        for (auto& t : unsent)
            _push_out(std::move(t));

        // wait until all the results get transmitted
        {
            std::unique_lock<std::mutex> ul(mtxDone);
            cvDone.wait(ul, 
                [this]
                {
                    return aActiveOut == 0;
                });
        }

        // stop transmitters
        aTransmitterStop = true;
        cvOut.notify_all();

        for (auto& transmitter : transmittingThreads)
            transmitter.join();
    }

private:

    void _push_out(TransmitInfo t)
    {
        {
            std::lock_guard<std::mutex> lg(mtxOut);
            qOut.push(std::move(t));
        }

        aActiveOut++;
        cvOut.notify_one();
    }

    void _submit_message_to_solve(msgId id, std::vector<u64> fragments)
    {
        auto foundFn = [this, id](msgId id_, const u8 data[], size_t bits)
        {
            assert(id == id_);
            
            CBigInt curr = countExpressions(data, bits);

            std::lock_guard<std::mutex> lg(mtxMsg);
            MessageState& msgSt = messages[id];
            if (!msgSt.result.has_value() || curr > msgSt.result.value())
                msgSt.result = curr;
        };

        auto finishedFn = [this, id](u32)
        {
            bool shouldSend = false;
            std::optional<CBigInt> val;
    
            {
                std::lock_guard<std::mutex> lg(mtxMsg);
                MessageState& msgSt = messages[id];
                msgSt.finished = true;
                
                // todo: probably not needed, but just to be sure
                if (!msgSt.sent)
                {
                    shouldSend = true;
                    msgSt.sent = true;
                    if (msgSt.result)
                        val = msgSt.result;
                }
            }

            if (shouldSend)
                _push_out({id, std::move(val)});

            aActiveProblems--;
            cvDone.notify_all();
        };

        aActiveProblems++;
        serializer.add_problem(std::move(fragments), std::move(foundFn), std::move(finishedFn));
    }

    void _receiver_job(AReceiver pReceiver)
    {
        u64 fragment;
        while (pReceiver->recv(fragment))
            addFragment(fragment);
    }

    void _worker_job()
    {
        std::function<void()> task;

        while (true)
        {
            {
                std::unique_lock<std::mutex> ul(mtxWork);
                cvWork.wait(ul, 
                    [this]
                    {
                        return aWorkerStop || !qTasks.empty();
                    });
                
                if (aWorkerStop && qTasks.empty())
                    return;

                task = std::move(qTasks.front());
                qTasks.pop();
            }

            task();
        }
    }

    void _transmitter_job(ATransmitter pTransmitter)
    {
        TransmitInfo task;
        while (true)
        {
            {
                std::unique_lock<std::mutex> ul(mtxOut);
                cvOut.wait(ul, 
                    [this]
                    {
                        return aTransmitterStop || !qOut.empty();
                    });

                if (aTransmitterStop && qOut.empty())
                    return;

                task = std::move(qOut.front());
                qOut.pop();
            }

            task.value.has_value() ? pTransmitter->send(task.id, *task.value) : 
                                     pTransmitter->incomplete(task.id);

            aActiveOut--;
            cvDone.notify_all();
        }
    }
};

#ifndef __PROGTEST__
int main()
{
    using namespace std::placeholders;

    msgSerializerLimits(100, 1, 1, 1, 1);
    for (const auto & x : g_TestSets)
    {
        CBigInt res;
        assert(CSentinelHacker::seqSolve(x.m_Fragments, res));
        assert(CBigInt(x.m_Result) == res);
    }

    msgSerializerLimits(4, 3, 5, 2, 2);

    CSentinelHacker test;
    auto            trans = std::make_shared<CExampleTransmitter>();
    AReceiver       recv  = std::make_shared<CExampleReceiver>(std::initializer_list<uint64_t>{0x508e000072ba, 0x508a000004a1, 0x788a0000058c, 0x246700000092});

    test.addTransmitter(trans);
    test.addReceiver(recv);
    test.start(3);

    static std::initializer_list<uint64_t> t1Data = {0x247300061fa2, 0x246d00003977, 0x5c8e000029aa, 0x5c890000009b};
    std::thread t1(fragmentSender, std::bind(&CSentinelHacker::addFragment, &test, _1), t1Data);

    static std::initializer_list<uint64_t> t2Data = {0x788d000036c6, 0x788e00002ab0, 0x508a0000036c, 0x246b00000e2b};
    std::thread t2(fragmentSender, std::bind(&CSentinelHacker::addFragment, &test, _1), t2Data);
    fragmentSender(std::bind(&CSentinelHacker::addFragment, &test, _1), std::initializer_list<uint64_t>{0x508d0000007f, 0x5c8b00000aab, 0x788e00007d7d, 0x508d00002f0b, 0x7893000e6648, 0x5c8f00009f2d});

    t1.join();
    t2.join();
    test.stop();

    assert(trans->totalSent() == 2);
    assert(trans->totalIncomplete() == 2);
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
