#pragma once

#include "resource.h"

// duilib
#include "duilib/duilib.h"

/** 工作线程
*/
class WorkerThread : public ui::FrameworkThread
{
public:
    WorkerThread();
    virtual ~WorkerThread(void);

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;
};

/** 主线程
*/
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread();

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;

private:
    /** 工作线程
    */
    std::unique_ptr<WorkerThread> m_workerThread;
};


