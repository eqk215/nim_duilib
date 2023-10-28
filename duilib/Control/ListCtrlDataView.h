#ifndef UI_CONTROL_LIST_CTRL_DATA_VIEW_H_
#define UI_CONTROL_LIST_CTRL_DATA_VIEW_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"

namespace ui
{
//�����ࣺListCtrlDataView / ListCtrlDataLayout

/** �б�������ʾ�Ͳ��ֹ���
*/
class ListCtrl;
class ListCtrlDataView : public VirtualListBox
{
    friend class ListCtrlDataLayout;
public:
    ListCtrlDataView();
    virtual ~ListCtrlDataView();

    /** ����ListCtrl�ؼ��ӿ�
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** ��ȡ�б��ؼ��Ŀ��ȣ�Header�ĸ������ܿ���֮�ͣ�
    */
    int32_t GetListCtrlWidth() const;

    /** ���ö���Ԫ�ص�������
    */
    void SetTopElementIndex(size_t nTopElementIndex);

    /** ��ȡ����Ԫ�ص�������
    */
    size_t GetTopElementIndex() const;

protected:
    /** ִ����ˢ�²���, �����UI�ؼ��������ܻᷢ���仯
    */
    virtual void OnRefresh() override;

    /** ִ�������Ų����������UI�ؼ�����������������䣨ͨ��FillElement������
    */
    virtual void OnArrangeChild() override;

private:
    /** ListCtrl �ؼ��ӿ�
    */
    ListCtrl* m_pListCtrl;

    /** ����Ԫ�ص�������(���ڻ�������)
    */
    size_t m_nTopElementIndex;
};

/** �б�������ʾ�ؼ��Ĳ��ֹ����ӿ�
*/
class ListCtrlDataLayout : public Layout, public VirtualLayout
{
public:
    /** �����ڲ����пؼ���λ����Ϣ
        * @param [in] items �ؼ��б�
        * @param[in] rc ��ǰ����λ����Ϣ, �����ڱ߾࣬����������߾�
        * @return �������к����պ��ӵĿ��Ⱥ͸߶���Ϣ
        */
    virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

    /** �����ڲ��ӿؼ���С��������������С���������͵��ӿؼ������ԣ��������С����
        * @param[in] items �ӿؼ��б�
        * @param [in] szAvailable ���ô�С������������ÿؼ����ڱ߾࣬��������������ؼ�����߾�
        * @return �������к����ղ��ֵĴ�С��Ϣ�����Ⱥ͸߶ȣ���
                ����items���ӿؼ�����߾࣬����items���ӿؼ����ڱ߾ࣻ
                ����Box�ؼ��������ڱ߾ࣻ
                ������Box�ؼ���������߾ࣻ
                ����ֵ�в������������͵��ӿؼ���С��
        */
    virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;

public:
    /** �ӳټ���չʾ����
        * @param [in] rc ��ǰ������С��Ϣ, �ⲿ����ʱ����Ҫ�ȼ�ȥ�ڱ߾�
        */
    virtual void LazyArrangeChild(UiRect rc) const override;

    /** ��ȡ��Ҫչʾ����ʵ������������������Control�����Ӧ����ʵ�����
    * @param [in] rc ��ǰ������С��Ϣ, �ⲿ����ʱ����Ҫ�ȼ�ȥ�ڱ߾�
    */
    virtual size_t AjustMaxItem(UiRect rc) const override;

    /** �õ��ɼ���Χ�ڵ�һ��Ԫ�ص�ǰһ��Ԫ������
    * @param [in] rc ��ǰ��ʾ����ľ��Σ��������ڱ߾�
    * @return ����Ԫ�ص�����
    */
    virtual size_t GetTopElementIndex(UiRect rc) const override;

    /** �ж�ĳ��Ԫ���Ƿ��ڿɼ���Χ��
    * @param[in] iIndex Ԫ������
    * @param [in] rc ��ǰ��ʾ����ľ��Σ��������ڱ߾�
    * @return ���� true ��ʾ�ɼ�������Ϊ���ɼ�
    */
    virtual bool IsElementDisplay(UiRect rc, size_t iIndex) const override;

    /** �ж��Ƿ�Ҫ���²���
    */
    virtual bool NeedReArrange() const override;

    /** ��ȡ��ǰ���пɼ��ؼ�������Ԫ������
    * @param [in] rc ��ǰ��ʾ����ľ��Σ��������ڱ߾�
    * @param[out] collection �����б�����Χ�ǣ�[0, GetElementCount())
    */
    virtual void GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const override;

    /** �ÿؼ��ڿɼ���Χ��
    * @param [in] rc ��ǰ��ʾ����ľ��Σ��������ڱ߾�
    * @param[in] iIndex Ԫ�������ţ���Χ�ǣ�[0, GetElementCount())
    * @param[in] bToTop �Ƿ������Ϸ�
    */
    virtual void EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const override;

private:
    /** ��ȡ������Box�ӿ�
    */
    ListCtrlDataView* GetOwnerBox() const;

    /** ��ȡ������ĸ߶�
    * @param [in] nCount ��������������ΪBox::InvalidIndex�����ȡ����������ĸ߶��ܺ�
    * @param [in] rc ��ǰ������С��Ϣ, �ⲿ����ʱ����Ҫ�ȼ�ȥ�ڱ߾�
    * @return ���� nCount ��������ĸ߶��ܺ�
    */
    int64_t GetElementsHeight(UiRect rc, size_t nCount) const;

    /** ��ȡ������ĸ߶ȺͿ���
    * @param [in] rc ��ǰ������С��Ϣ, �ⲿ����ʱ����Ҫ�ȼ�ȥ�ڱ߾�
    * @param [in] nElementIndex ����Ԫ�ص�������
    * @return ��������Ԫ�صĸ߶ȺͿ���
    */
    UiSize GetElementSize(UiRect rc, size_t nElementIndex) const;

    /** ��������
    *@param [in] rcWidth �����������
    *@return ����õ�������, ���ڻ����1
    */
    int32_t CalcTileColumns(int32_t rcWidth) const;

    /** ��ȡ�п�
    */
    int32_t GetItemWidth() const;

    /** ��ȡ�и�(Ŀǰ��֧�������е��и߶���ȵ����)
    */
    int32_t GetItemHeight() const;

    /** ��ȡ��ͷ�ؼ��ĸ߶�
    */
    int32_t GetHeaderHeight() const;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DATA_VIEW_H_