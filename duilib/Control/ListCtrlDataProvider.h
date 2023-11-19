#ifndef UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
#define UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"
#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表项的数据管理器
*/
class ListCtrl;
struct ListCtrlSubItemData;
class ListCtrlDataProvider : public ui::VirtualListBoxElement
{
public:
    //用于存储的数据结构
    typedef ListCtrlSubItemData2 Storage;
    typedef std::shared_ptr<Storage> StoragePtr;
    typedef std::vector<StoragePtr> StoragePtrList;
    typedef std::unordered_map<size_t, StoragePtrList> StorageMap;
    typedef std::vector<ListCtrlItemData> RowDataList;

public:
    ListCtrlDataProvider();

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateElement() override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override;

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() const override;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) const override;

    /** 获取选择的元素列表
    * @param [in] selectedIndexs 返回当前选择的元素列表，有效范围：[0, GetElementCount())
    */
    virtual void GetSelectedElements(std::vector<size_t>& selectedIndexs) const override;

    /** 是否支持多选
    */
    virtual bool IsMultiSelect() const override;

    /** 设置是否支持多选，由界面层调用，保持与界面控件一致
    * @return bMultiSelect true表示支持多选，false表示不支持多选
    */
    virtual void SetMultiSelect(bool bMultiSelect) override;

public:
    /** 设置表头接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 增加一列
    * @param [in] columnId 列的ID
    */
    bool AddColumn(size_t columnId);

    /** 删除一列
    * @param [in] columnId 列的ID
    */
    bool RemoveColumn(size_t columnId);

    /** 获取某列的宽度最大值
    * @return 返回该列宽度的最大值，返回的是DPI自适应后的值； 如果失败返回-1
    */
    int32_t GetColumnWidthAuto(size_t columnId) const;

    /** 设置一列的勾选状态（Checked或者UnChecked）
    * @param [in] columnId 列的ID
    * @param [in] bChecked true表示选择，false表示取消选择
    */
    bool SetColumnCheck(size_t columnId, bool bChecked);

    /** 获取数据项总个数
    */
    size_t GetDataItemCount() const;

    /** 设置数据项总个数
    * @param [in] itemCount 数据项的总数，具体每个数据项的数据，通过回调的方式进行填充（内部为虚表实现）
    */
    bool SetDataItemCount(size_t itemCount);

    /** 在最后添加一个数据项
    * @param [in] dataItem 数据项的内容
    * @return 成功返回数据项的行索引号，失败则返回Box::InvalidIndex
    */
    size_t AddDataItem(const ListCtrlSubItemData& dataItem);

    /** 在指定行位置添加一个数据项
    * @param [in] itemIndex 数据项的索引号
    * @param [in] dataItem 数据项的内容
    */
    bool InsertDataItem(size_t itemIndex, const ListCtrlSubItemData& dataItem);

    /** 设置指定<行,列>的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] dataItem 指定数据项的内容，列序号在dataItem.nColumnIndex中指定
    */
    bool SetDataItem(size_t itemIndex, const ListCtrlSubItemData& dataItem);

    /** 获取指定<行,列>的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] dataItem 指定数据项的内容
    */
    bool GetDataItem(size_t itemIndex, size_t columnIndex, ListCtrlSubItemData& dataItem) const;

    /** 删除指定行的数据项
    * @param [in] itemIndex 数据项的索引号
    */
    bool DeleteDataItem(size_t itemIndex);

    /** 删除所有行的数据项
    */
    bool DeleteAllDataItems();

    /** 设置数据项的行属性数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 关联的数据
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData, bool& bChanged);

    /** 获取数据项的行属性数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 关联的数据
    */
    bool GetDataItemData(size_t itemIndex, ListCtrlItemData& itemData) const;

    /** 设置数据项的可见性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bVisible 是否可见
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemVisible(size_t itemIndex, bool bVisible, bool& bChanged);

    /** 获取数据项的可见性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的可见性
    */
    bool IsDataItemVisible(size_t itemIndex) const;

    /** 设置数据项的选择属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bSelected 是否选择状态
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemSelected(size_t itemIndex, bool bSelected, bool& bChanged);

    /** 获取数据项的选择属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的选择状态
    */
    bool IsDataItemSelected(size_t itemIndex) const;

    /** 设置行首的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] imageId 图标资源Id，如果为-1表示行首不显示图标, 该ID由ImageList生成
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemImageId(size_t itemIndex, int32_t imageId, bool& bChanged);

    /** 获取行首的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    int32_t GetDataItemImageId(size_t itemIndex) const;

    /** 获取选择状态(bSelect)
    * @param [out] bSelected 是否选择
    * @param [out] bPartSelected 是否部分选择
    */
    void GetDataItemsSelectStatus(bool& bSelected, bool& bPartSelected) const;

    /** 设置数据项的勾选属性（每行前面的CheckBox）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bChecked 是否勾选状态
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemChecked(size_t itemIndex, bool bChecked, bool& bChanged);

    /** 获取数据项的选择属性（每行前面的CheckBox）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的勾选状态
    */
    bool IsDataItemChecked(size_t itemIndex) const;

    /** 设置所有行的勾选状态（Checked或者UnChecked）
    * @param [in] bChecked true表示选择，false表示取消选择
    */
    bool SetAllDataItemsCheck(bool bChecked);

    /** 批量设置勾选数据项（行首的CheckBox打勾的数据）
    * @param [in] itemIndexs 需要设置勾选的数据项索引号，有效范围：[0, GetDataItemCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SetCheckedDataItems(const std::vector<size_t>& itemIndexs,
                             bool bClearOthers,
                             std::vector<size_t>& refreshIndexs);

    /** 获取勾选的元素列表（行首的CheckBox打勾的数据）
    * @param [in] itemIndexs 返回当前勾选的数据项索引号，有效范围：[0, GetDataItemCount())
    */
    void GetCheckedDataItems(std::vector<size_t>& itemIndexs) const;

    /** 获取勾选状态(bChecked)
    * @param [out] bChecked 是否勾选
    * @param [out] bPartChecked 是否部分勾选
    */
    void GetDataItemsCheckStatus(bool& bChecked, bool& bPartChecked) const;

    /** 设置数据项的置顶状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nAlwaysAtTop 置顶状态，-1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop, bool& bChanged);

    /** 获取数据项的置顶状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的置顶状态，-1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    */
    int8_t GetDataItemAlwaysAtTop(size_t itemIndex) const;

    /** 设置数据项的行高
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nItemHeight 行高, -1表示使用ListCtrl设置的默认行高，其他值表示本行的设置行高
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale, bool& bChanged);

    /** 获取数据项的行高
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的行高, -1表示使用ListCtrl设置的默认行高，其他值表示本行的设置行高
    */
    int32_t GetDataItemHeight(size_t itemIndex) const;

    /** 设置数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号
    * @param [in] userData 数据项关联的自定义数据
    */
    bool SetDataItemUserData(size_t itemIndex, size_t userData);

    /** 获取数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号
    * @return 返回数据项关联的自定义数据
    */
    size_t GetDataItemUserData(size_t itemIndex) const;

public:
    /** 设置指定数据项的文本
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] text 需要设置的文本内容
    */
    bool SetDataItemText(size_t itemIndex, size_t columnIndex, const std::wstring& text);

    /** 获取指定数据项的文本
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 数据项关联的文本内容
    */
    std::wstring GetDataItemText(size_t itemIndex, size_t columnIndex) const;

    /** 设置指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] textColor 需要设置的文本颜色
    */
    bool SetDataItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor);

    /** 获取指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] textColor 数据项关联的文本颜色
    */
    bool GetDataItemTextColor(size_t itemIndex, size_t columnIndex, UiColor& textColor) const;

    /** 设置指定数据项的文本属性（文本对齐方式等）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] nTextFormat 需要设置的文本属性
    */
    bool SetDataItemTextFormat(size_t itemIndex, size_t columnIndex, int32_t nTextFormat);

    /** 获取指定数据项的文本属性（文本对齐方式等）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 数据项关联的文本属性
    */
    int32_t GetDataItemTextFormat(size_t itemIndex, size_t columnIndex) const;

    /** 设置指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bkColor 需要设置的背景颜色
    */
    bool SetDataItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor);

    /** 获取指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] bkColor 数据项关联的背景颜色
    */
    bool GetDataItemBkColor(size_t itemIndex, size_t columnIndex, UiColor& bkColor) const;

    /** 是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    */
    bool IsShowCheckBox(size_t itemIndex, size_t columnIndex) const;

    /** 设置是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bShowCheckBox true表示显示，false表示不显示
    */
    bool SetShowCheckBox(size_t itemIndex, size_t columnIndex, bool bShowCheckBox);

    /** 设置CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bSelected true表示勾选，false表示不勾选
    */
    bool SetCheckBoxCheck(size_t itemIndex, size_t columnIndex, bool bSelected);

    /** 获取CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] bSelected true表示勾选，false表示不勾选
    */
    bool GetCheckBoxCheck(size_t itemIndex, size_t columnIndex, bool& bSelected) const;

    /** 获取选择状态(bSelect)
    * @param [in] columnId 列的ID
    * @param [out] bChecked 是否选择
    * @param [out] bPartChecked 是否部分选择
    */
    void GetCheckBoxCheckStatus(size_t columnId, bool& bChecked, bool& bPartChecked) const;

    /** 设置该列的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] imageId 图标资源Id，如果为-1表示行首不显示图标, 该ID由ImageList生成
    */
    bool SetDataItemImageId(size_t itemIndex, size_t columnIndex, int32_t imageId);

    /** 获取该列的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    */
    int32_t GetDataItemImageId(size_t itemIndex, size_t columnIndex) const;

    /** 对数据排序
    * @param [in] columnId 列的ID
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortDataItems(size_t nColumnId, bool bSortedUp,
                       ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

    /** 设置外部自定义的排序函数, 替换默认的排序函数
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    void SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

private:
    /** 数据转换为存储数据结构
    */
    void DataItemToStorage(const ListCtrlSubItemData& item, Storage& storage) const;

    /** 存储数据转换为结构数据
    */
    void StorageToDataItem(const Storage& storage, ListCtrlSubItemData& item) const;

    /** 根据列序号查找列ID
    * @return 返回列ID，如果匹配不到，则返回Box::InvalidIndex
    */
    size_t GetColumnId(size_t nColumnIndex) const;

    /** 根据列ID查找列序号
    * @param [in] nColumnId 列ID
    * @return 返回列序号，如果匹配不到，则返回Box::InvalidIndex
    */
    size_t GetColumnIndex(size_t nColumnId) const;

    /** 判断一个数据项索引是否有效
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    bool IsValidDataItemIndex(size_t itemIndex) const;

    /** 判断一个列ID在数据存储中是否有效
    */
    bool IsValidDataColumnId(size_t nColumnId) const;

    /** 获取指定数据项的数据, 读取
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 如果失败则返回nullptr
    */
    StoragePtr GetDataItemStorage(size_t itemIndex, size_t columnIndex) const;

    /** 获取指定数据项的数据, 写入
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 如果失败则返回nullptr
    */
    StoragePtr GetDataItemStorageForWrite(size_t itemIndex, size_t columnIndex);

    /** 获取各个列的数据，用于UI展示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIdList 列ID列表
    * @param [out] storageList 返回数据列表
    */
    bool GetDataItemStorageList(size_t itemIndex,
                                std::vector<size_t>& columnIdList,
                                StoragePtrList& storageList) const;

    /** 某个数据项的Check勾选状态变化(列级)
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列ID
    * @param [in] bChecked 是否勾选
    */
    void OnDataItemColumnChecked(size_t itemIndex, size_t nColumnId, bool bChecked);

    /** 同步UI的Check状态(列级别的CheckBox)
    * @param [in] nColumnId 列ID
    */
    void UpdateHeaderColumnCheckBox(size_t nColumnId);

public:
    /** 获取行属性数据
    */
    const RowDataList& GetItemDataList() const;

    /** 是否为标准模式（行高都为默认行高，无隐藏行，无置顶行）
    */
    bool IsNormalMode() const;

private:
    /** 排序数据
    */
    struct StorageData
    {
        size_t index;       //原来的数据索引号
        StoragePtr pStorage;
    };

    /** 对数据排序
    * @param [in] dataList 待排序的数据
    * @param [in] nColumnId 列的ID
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortStorageData(std::vector<StorageData>& dataList,
                         size_t nColumnId, bool bSortedUp,
                         ListCtrlDataCompareFunc pfnCompareFunc,
                         void* pUserData);

    /** 默认的数据比较函数
    * @param [in] a 第一个比较数据
    * @param [in] b 第二个比较数据
    * @return 如果 (a < b)，返回true，否则返回false
    */
    bool SortDataCompareFunc(const ListCtrlSubItemData2& a, const ListCtrlSubItemData2& b) const;

    /** 更新个性化数据（隐藏行、行高、置顶等）
    */
    void UpdateNormalMode();

private:
    /** 表头控件
    */
    ListCtrl* m_pListCtrl;

    /** 数据，按列保存，每个列一个数组
    */
    StorageMap m_dataMap;

    /** 行的属性数据
    */
    RowDataList m_rowDataList;

    /** 外部设置的排序函数
    */
    ListCtrlDataCompareFunc m_pfnCompareFunc;

    /** 外部设置的排序函数附加数据
    */
    void* m_pUserData;

    /** 隐藏行的个数
    */
    int32_t m_hideRowCount;

    /** 非默认行高行的个数
    */
    int32_t m_heightRowCount;

    /** 置顶行的个数
    */
    int32_t m_atTopRowCount;

    /** 是否支持多选
    */
    bool m_bMultiSelect;

    /** 单选的时候，选择的元素索引号
    */
    size_t m_nSelectedIndex;

    /** 当前默认的文本属性
    */
    int32_t m_nDefaultTextStyle;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
