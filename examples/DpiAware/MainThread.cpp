//MainThread.cpp
#include "MainThread.h"
#include "MainForm.h"

WorkerThread::WorkerThread()
    : FrameworkThread(_T("WorkerThread"), ui::kThreadWorker)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::OnInit()
{
    ::OleInitialize(nullptr);
}

void WorkerThread::OnCleanup()
{
    ::OleUninitialize();
}

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

void MainThread::OnInit()
{
    ::OleInitialize(nullptr);

    //启动工作线程
    m_workerThread.reset(new WorkerThread);
    m_workerThread->Start();

    //初始化全局资源, 使用本地文件夹作为资源
    DString resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //在下面加入启动窗口代码
    //
    //创建一个默认带有阴影的居中窗口
    MainForm* window = new MainForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwExStyle = WS_EX_LAYERED;
    createParam.m_className = _T("DpiAware");
    createParam.m_windowTitle = createParam.m_className;
    window->CreateWnd(nullptr, createParam);
    window->PostQuitMsgWhenClosed(true);
    window->CenterWindow();
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
    if (m_workerThread != nullptr) {
        m_workerThread->Stop();
        m_workerThread.reset(nullptr);
    }
    ::OleUninitialize();
}
