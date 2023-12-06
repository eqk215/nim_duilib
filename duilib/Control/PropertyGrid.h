#ifndef UI_CONTROL_PROPERTY_GRID_H_
#define UI_CONTROL_PROPERTY_GRID_H_

#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Control/Split.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/RichText.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/TreeView.h"
#include "duilib/Control/Combo.h"

namespace ui
{
/** 属性表控件
*/
class PropertyGridGroup;
class PropertyGridProperty;
class PropertyGridTextProperty;
class PropertyGridComboProperty;
class PropertyGrid : public VBox
{
public:
	PropertyGrid();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
	/** 设置是否显示表头
	*/
	void SetEnableHeaderCtrl(bool bEnable,
							 const std::wstring& sLeftColumn = L"", 
							 const std::wstring& sRightColumn = L"");
	/** 判断当前是否显示表头
	*/
	bool IsEnableHeaderCtrl() const { return m_bHeaderCtrl; }

	/** 获取Header接口
	*/
	Control* GetHeaderCtrl() const { return m_pHeaderCtrl; }

	/** 获取Header中的左侧一列
	*/
	Label* GetHeaderLeft() const { return m_pHeaderLeft; }

	/** 获取Header中的右侧一列
	*/
	Label* GetHeaderRight() const { return m_pHeaderRight; }

	/** 获取Header中的分割条
	*/
	Split* GetHeaderSplit() const { return m_pHeaderSplit; }

public:
	/** 增加一个分组
	* @param [in] groupName 分组的名称
	* @param [in] description 分组的描述信息
	* @param [in] nGroupData 用户自定义数据
	* @return 返回该分组的接口，可用于添加属性
	*/
	PropertyGridGroup* AddGroup(const std::wstring& groupName, 
							    const std::wstring& description = L"",
								size_t nGroupData = 0);

	/** 获取所有的分组
	* @param [out] groups 返回当前所有的分组列表
	*/
	void GetGroups(std::vector<PropertyGridGroup*>& groups) const;

	/** 删除分组
	* @param [in] pGroup 待删除的分组
	*/
	bool RemoveGroup(PropertyGridGroup* pGroup);

	/** 删除所有分组
	*/
	void RemoveAllGroups();

	/** 添加一个属性(由调用方创建属性)
	* @param [in] pGroup 该属性所属的分组
	* @param [in] pProperty 属性接口
	*/
	bool AddProperty(PropertyGridGroup* pGroup, PropertyGridProperty* pProperty);

	/** 增加一个属性
	* @param [in] pGroup 该属性所属的分组
	* @param [in] propertyName 属性的名称
	* @param [in] propertyValue 属性的值
	* @param [in] description 属性的描述信息
	* @param [in] nPropertyData 用户自定义数据
	* @return 返回该属性的接口
	*/
	PropertyGridTextProperty* AddTextProperty(PropertyGridGroup* pGroup,
									          const std::wstring& propertyName, 
									          const std::wstring& propertyValue,
									          const std::wstring& description = L"",
									          size_t nPropertyData = 0);

	/** 增加一个属性
	* @param [in] pGroup 该属性所属的分组
	* @param [in] propertyName 属性的名称
	* @param [in] propertyValue 属性的值
	* @param [in] description 属性的描述信息
	* @param [in] nPropertyData 用户自定义数据
	* @return 返回该属性的接口
	*/
	PropertyGridComboProperty* AddComboProperty(PropertyGridGroup* pGroup,
									            const std::wstring& propertyName, 
									            const std::wstring& propertyValue,
									            const std::wstring& description = L"",
									            size_t nPropertyData = 0);

	/** 设置左侧一列的宽度
	* @param [in] nLeftColumnWidth 左侧一列的宽度
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
	*/
	void SetLeftColumnWidth(int32_t nLeftColumnWidth, bool bNeedDpiScale);

	/** 获取左侧一列的宽度值
	*/
	int32_t GetLeftColumnWidth() const;

public:
	/** 设置是否显示描述区域
	*/
	void SetEnableDescriptionArea(bool bEnable);

	/** 判断是否显示描述区域
	*/
	bool IsEnableDescriptionArea() const { return m_bDescriptionArea; }

	/** 设置描述区域的高度值
	* @param [in] nHeight 高度值
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
	*/
	void SetDescriptionAreaHeight(int32_t nHeight, bool bNeedDpiScale);

	/** 获取当前描述区域的高度值
	*/
	int32_t GetDescriptionAreaHeight() const;

	/** 获取描述控件的接口
	*/
	RichText* GetDescriptionArea() const { return m_pDescriptionArea; }

	/** 描述控件的分割条接口
	*/
	Split* GetDescriptionAreaSplit() const { return m_pDescriptionAreaSplit; }

public:
	/** 获取属性表的树控件接口（用于管理数据）
	*/
	TreeView* GetTreeView() const { return m_pTreeView; }

	/** 横向网格线的宽度
	* @param [in] nLineWidth 网格线的宽度，如果为0表示不显示横向网格线
	* @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
	*/
	void SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
	int32_t GetRowGridLineWidth() const;

	/** 横向网格线的颜色
	* @param [in] color 横向网格线的颜色
	*/
	void SetRowGridLineColor(const std::wstring& color);
	std::wstring GetRowGridLineColor() const;

	/** 纵向网格线的宽度
	* @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
	* @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
	*/
	void SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
	int32_t GetColumnGridLineWidth() const;

	/** 纵向网格线的颜色
	* @param [in] color 纵向网格线的颜色
	*/
	void SetColumnGridLineColor(const std::wstring& color);
	std::wstring GetColumnGridLineColor() const;

	/** 表头的Class
	*/
	void SetHeaderClass(const std::wstring& headerClass);
	std::wstring GetHeaderClass() const;

	/** 分组的Class
	*/
	void SetGroupClass(const std::wstring& groupClass);
	std::wstring GetGroupClass() const;

	/** 属性的Class
	*/
	void SetPropertyClass(const std::wstring& propertyClass);
	std::wstring GetPropertyClass() const;

protected:
	/** 初始化函数
	 */
	virtual void DoInit() override;

	/** 绘制子控件
	*/
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

	/** 绘制网格线
	*/
	void PaintGridLines(IRender* pRender);

	/** 拖动列表头改变列宽的事件响应函数
	*/
	void OnHeaderColumnResized();

	/** 调整一个属性节点的列宽
	*/
	void ResizePropertyColumn(TreeNode* pPropertyNode, int32_t nLeftColumnWidth);

private:
	/** 是否已经完成初始化
	*/
	bool m_bInited;

	/** 配置XML文件
	*/
	UiString m_configXml;

private:
	/** 是否显示Header
	*/
	bool m_bHeaderCtrl;

	/** Header接口
	*/
	Control* m_pHeaderCtrl;

	/** Header中的左侧一列
	*/
	Label* m_pHeaderLeft;

	/** Header中的右侧一列
	*/
	Label* m_pHeaderRight;

	/** Header中的分割条
	*/
	Split* m_pHeaderSplit;

	/** 左侧一列的宽度
	*/
	int32_t m_nLeftColumnWidth;

private:
	/** 描述控件的分割条
	*/
	Split* m_pDescriptionAreaSplit;

	/** 描述控件
	*/
	RichText* m_pDescriptionArea;

	/** 是否显示描述区域
	*/
	bool m_bDescriptionArea;

private:
	/** 属性表的树控件接口
	*/
	TreeView* m_pTreeView;

	/** 表头的Class
	*/
	UiString m_headerClass;

	/** 分组的Class
	*/
	UiString m_groupClass;

	/** 属性的Class
	*/
	UiString m_propertyClass;

private:
	/** 横向网格线的宽度
	*/
	int32_t m_nRowGridLineWidth;

	/** 横向网格线的颜色
	*/
	UiString m_rowGridLineColor;

	/** 纵向网格线的宽度
	*/
	int32_t m_nColumnGridLineWidth;

	/** 纵向网格线的颜色
	*/
	UiString m_columnGridLineColor;
};

/** 属性表的分组, 基本结构
*   <PropertyGridGroup>
*		<HBox>
*			<LabelBox/>
*		</HBox>
*   </PropertyGridGroup>
*/
class PropertyGridGroup : public TreeNode
{
public:
	/** 构造一个组
	@param [in] groupName 组的名称
	@param [in] description 组的描述信息
	@param [in] nGroupData 用户自定义数据
	*/
	explicit PropertyGridGroup(const std::wstring& groupName, 
							   const std::wstring& description = L"",
		                       size_t nGroupData = 0);

public:
	/** 获取属性名称
	*/
	std::wstring GetGroupName() const { return m_groupName.c_str(); }

	/** 获取组的描述信息
	*/
	std::wstring GetDescriptiion() const { return m_description.c_str(); }

	/** 获取用户自定义数据
	*/
	size_t GetGroupData() const { return m_nGroupData; }

	/** 设置用户自定义数据
	*/
	void SetGroupData(size_t nGroupData) { m_nGroupData = nGroupData; }

	/** 获取属性名称的显示控件
	*/
	LabelBox* GetLabelBox() const { return m_pLabelBox; }

	/** 获取分组下的所有属性
	* @param [out] properties 返回当前所有的分组列表
	*/
	void GetProperties(std::vector<PropertyGridProperty*>& properties) const;

	/** 删除该分组下的属性
	* @param [in] pProperty 需要删除的属性接口
	*/
	bool RemoveProperty(PropertyGridProperty* pProperty);

	/** 删除该分组下的所有属性
	*/
	void RemoveAllProperties();

protected:
	/** 初始化函数
	 */
	virtual void DoInit() override;

private:
	/** 是否已经完成初始化
	*/
	bool m_bInited;

	/** 分组的名称
	*/
	UiString m_groupName;

	/** 组的描述信息
	*/
	UiString m_description;

	/** 用户自定义数据
	*/
	size_t m_nGroupData;

	/** 属性名称的显示控件
	*/
	LabelBox* m_pLabelBox;
};

/** 属性的类型
*/
enum class PropertyGridPropertyType
{
	kNone,		//无具体类型，基类
	kText,		//普通文本
	kCombo,		//下拉框

	kCustom		//用户自定义的类型，比如自己实现一个子类
};

/** 属性表的属性, 基本结构
*   <PropertyGridProperty>
*		<HBox>
*			<LabelBox/>
*			<LabelBox/>
*		</HBox>
*   </PropertyGridProperty>
*/
class PropertyGridProperty: public TreeNode
{
public:
	/** 构造一个属性
	@param [in] propertyName 属性的名称
	@param [in] propertyValue 属性的值
	@param [in] description 属性的描述信息
	@param [in] nPropertyData 用户自定义数据
	*/
	PropertyGridProperty(const std::wstring& propertyName, 
					     const std::wstring& propertyValue,
					     const std::wstring& description = L"",
					     size_t nPropertyData = 0);

public:
	/** 获取属性类型
	*/
	virtual PropertyGridPropertyType GetPropertyType() const 
	{ 
		return PropertyGridPropertyType::kNone; 
	}

	/** 获取属性名称
	*/
	std::wstring GetPropertyName() const { return m_propertyName.c_str(); }

	/** 获取属性值(原值)
	*/
	std::wstring GetPropertyValue() const { return m_propertyValue.c_str(); }

	/** 获取属性的描述信息
	*/
	std::wstring GetDescriptiion() const { return m_description.c_str(); }

	/** 获取用户自定义数据
	*/
	size_t GetPropertyData() const { return m_nPropertyData; }

	/** 设置用户自定义数据
	*/
	void SetPropertyData(size_t nPropertyData) { m_nPropertyData = nPropertyData; }

	/** 获取属性名称和属性值所在容器控件，可用于设置背景色等
	*/
	HBox* GetHBox() const {	return m_pHBox;	}

	/** 获取属性名称的显示控件, 父控件是GetHBox()
	*/
	LabelBox* GetLabelBoxLeft() const { return m_pLabelBoxLeft; }

	/** 获取属性值的显示控件, 父控件是GetHBox()
	*/
	LabelBox* GetLabelBoxRight() const { return m_pLabelBoxRight; }

	/** 设置只读模式
	*/
	void SetReadOnly(bool bReadOnly);

	/** 是否为只读模式
	*/
	bool IsReadOnly() const { return m_bReadOnly; }

	/** 获取新的属性值（修改后的属性值, 如果无修改则返回原值）
	*/
	virtual std::wstring GetPropertyNewValue() const;

protected:
	/** 是否已经初始化
	*/
	bool IsInited() const { return m_bInited; }

	/** 初始化函数
	 */
	virtual void DoInit() override;

	/** 设置是否允许存在编辑框控件
	* @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
	*/
	virtual void EnableEditControl(bool /*bEnable*/) {}

	/** 显示或者隐藏编辑框控件
	* @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
	*/
	virtual void ShowEditControl(bool /*bShow*/) {}

protected:
	/** 设置属性值的文本(显示控件)
	* @param [in] text 文本内容
	* @param [in] bChanged 是否标记为变化
	*/
	void SetPropertyText(const std::wstring& text, bool bChanged);

	/** 获取属性值文本(显示控件)
	*/
	std::wstring GetPropertyText() const;

	/** 将焦点设置到属性值文本显示控件
	*/
	void SetPropertyFocus();

	/** 在属性值的LabelBox中添加控件
	*/
	bool AddPropertySubItem(Control* pControl);

	/** 在属性值的LabelBox中移除控件
	*/
	bool RemovePropertySubItem(Control* pControl);

	/** 判断属性值的LabelBox中是否包含控件
	*/
	bool HasPropertySubItem(Control* pControl) const;

private:
	/** 是否已经完成初始化
	*/
	bool m_bInited;

	/** 属性的名称
	*/
	UiString m_propertyName;

	/** 属性的值
	*/
	UiString m_propertyValue;

	/** 属性的描述信息
	*/
	UiString m_description;

	/** 用户自定义数据
	*/
	size_t m_nPropertyData;

	/** 属性名称和属性值所在容器控件
	*/
	HBox* m_pHBox;

	/** 属性名称的显示控件
	*/
	LabelBox* m_pLabelBoxLeft;

	/** 属性值的显示控件
	*/
	LabelBox* m_pLabelBoxRight;

	/** 只读模式
	*/
	bool m_bReadOnly;
};

/** 文本类型的属性：使用RichEdit编辑
*/
class PropertyGridTextProperty : public PropertyGridProperty
{
public:
	/** 构造一个属性
	@param [in] propertyName 属性的名称
	@param [in] propertyValue 属性的值
	@param [in] description 属性的描述信息
	@param [in] nPropertyData 用户自定义数据
	*/
	PropertyGridTextProperty(const std::wstring& propertyName,
					         const std::wstring& propertyValue,
					         const std::wstring& description = L"",
					         size_t nPropertyData = 0);

public:
	/** 获取属性类型
	*/
	virtual PropertyGridPropertyType GetPropertyType() const
	{
		return PropertyGridPropertyType::kText;
	}

	/** 获取新的属性值（修改后的属性值, 如果无修改则返回原值）
	*/
	virtual std::wstring GetPropertyNewValue() const override;

	/** 获取编辑框控件
	*/
	RichEdit* GetRichEdit() const { return m_pRichEdit; }

	/** 设置密码模式（显示 ***）
	 * @param[in] bPassword 设置为 true 让控件显示内容为 ***，false 为显示正常内容
	 */
	void SetPassword(bool bPassword);

	/** 是否为密码模式
	*/
	bool IsPassword() const { return m_bPassword; }

	/** 设置是否支持Spin控件
	* @param [in] bEnable true表示支持Spin控件，false表示不支持Spin控件
	* @param [in] nMin 表示设置数字的最小值
	* @param [in] nMax 表示设置数字的最大值，如果 nMin和nMax同时为0, 表示不设置数字的最小值和最大值
	*/
	void SetEnableSpin(bool bEnable, int32_t nMin = 0, int32_t nMax = 0);

protected:
	/** 设置是否允许存在编辑框控件
	* @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
	*/
	virtual void EnableEditControl(bool bEnable) override;

	/** 显示或者隐藏编辑框控件
	* @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
	*/
	virtual void ShowEditControl(bool bShow) override;

private:
	/** 编辑框控件(用于修改属性)
	*/
	RichEdit* m_pRichEdit;

	/** 密码模式
	*/
	bool m_bPassword;
};

/** 下拉框类型的属性：使用Combo编辑
*/
class PropertyGridComboProperty : public PropertyGridProperty
{
public:
	/** 构造一个属性
	@param [in] propertyName 属性的名称
	@param [in] propertyValue 属性的值
	@param [in] description 属性的描述信息
	@param [in] nPropertyData 用户自定义数据
	*/
	PropertyGridComboProperty(const std::wstring& propertyName,
							  const std::wstring& propertyValue,
							  const std::wstring& description = L"",
							  size_t nPropertyData = 0);

public:
	/** 获取属性类型
	*/
	virtual PropertyGridPropertyType GetPropertyType() const
	{
		return PropertyGridPropertyType::kCombo;
	}

	/** 获取新的属性值（修改后的属性值, 如果无修改则返回原值）
	*/
	virtual std::wstring GetPropertyNewValue() const override;

	/** 增加一个下拉框选项
	* @param [in] optionText 下拉框列表项的内容
	*/
	void AddOption(const std::wstring& optionText);

	/** 获取下拉框选项的格式
	*/
	size_t GetOptionCount() const;

	/** 获取下拉表子项的文本
	* @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
	*/
	std::wstring GetOption(size_t nIndex) const;

	/** 删除指定的子项
	* @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
	*/
	bool RemoveOption(size_t nIndex);

	/** 删除所有子项
	*/
	void RemoveAllOptions();

	/** 获取当前选择项索引
	 * @return 返回当前选择项索引, (如果无有效索引，则返回Box::InvalidIndex)
	 */
	size_t GetCurSel() const;

	/** 选择一个子项, 不触发选择事件
	 * @param[in] nIndex 要选择的子项索引，有效范围：[0, GetOptionCount())
	 * @return 返回 true 表示成功，否则为 false
	 */
	bool SetCurSel(size_t nIndex);

	/** 设置为列表模式
	* @param [in] bListMode true表示不支持编辑文本，只能从下拉表中选择；false表示允许编辑，允许选择
	*/
	void SetComboListMode(bool bListMode);

	/** 获取下拉框接口
	*/
	Combo* GetCombo() const { return m_pCombo; }

protected:
	/** 设置是否允许存在编辑框控件
	* @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
	*/
	virtual void EnableEditControl(bool bEnable) override;

	/** 显示或者隐藏编辑框控件
	* @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
	*/
	virtual void ShowEditControl(bool bShow) override;

private:
	/** 下拉框接口
	*/
	Combo* m_pCombo;
};

}//namespace ui

#endif //UI_CONTROL_PROPERTY_GRID_H_
