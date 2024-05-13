//MainForm.h
#ifndef MAIN_FORM_H_
#define MAIN_FORM_H_

#pragma once

// duilib
#include "duilib/duilib.h"

/** Ӧ�ó����������ʵ��
*/
class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm();

    /**  ��������ʱ�����ã�������ʵ�����Ի�ȡ����Ƥ��Ŀ¼
    * @return ������ʵ�ֲ����ش���Ƥ��Ŀ¼
    */
    virtual std::wstring GetSkinFolder() override;

    /**  ��������ʱ�����ã�������ʵ�����Ի�ȡ����Ƥ�� XML �����ļ�
    * @return ������ʵ�ֲ����ش���Ƥ�� XML �����ļ�
    *         ���ص����ݣ�������XML�ļ����ݣ����ַ�'<'Ϊ��ʼ���ַ�������
    *         �������ļ�·����������'<'�ַ���ʼ���ַ��������ļ�Ҫ��GetSkinFolder()·�����ܹ��ҵ�
    */
    virtual std::wstring GetSkinFile() override;

    /** ��������ʱ�����ã�������ʵ�����Ի�ȡ����Ψһ��������
    * @return ������ʵ�ֲ����ش���Ψһ��������
    */
    virtual std::wstring GetWindowClassName() const override;

    /** �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
     */
    virtual void OnInitWindow() override;

    /** �յ� WM_CLOSE ��Ϣʱ�ú����ᱻ����
     */
    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** ���ڵ�����
    */
    static const std::wstring kClassName;
};

#endif //MAIN_FORM_H_