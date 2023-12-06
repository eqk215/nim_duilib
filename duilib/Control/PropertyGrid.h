#ifndef UI_CONTROL_PROPERTY_GRID_H_
#define UI_CONTROL_PROPERTY_GRID_H_

#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Control/Split.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/RichText.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/TreeView.h"

namespace ui
{
/** ���Ա��ؼ�
*/
class PropertyGridGroup;
class PropertyGridProperty;
class PropertyGrid : public VBox
{
public:
	PropertyGrid();

	/** ��ȡ�ؼ�����
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
	/** �����Ƿ���ʾ��ͷ
	*/
	void SetEnableHeaderCtrl(bool bEnable,
							 const std::wstring& sLeftColumn = L"", 
							 const std::wstring& sRightColumn = L"");
	/** �жϵ�ǰ�Ƿ���ʾ��ͷ
	*/
	bool IsEnableHeaderCtrl() const { return m_bHeaderCtrl; }

	/** ��ȡHeader�ӿ�
	*/
	Control* GetHeaderCtrl() const { return m_pHeaderCtrl; }

	/** ��ȡHeader�е����һ��
	*/
	Label* GetHeaderLeft() const { return m_pHeaderLeft; }

	/** ��ȡHeader�е��Ҳ�һ��
	*/
	Label* GetHeaderRight() const { return m_pHeaderRight; }

	/** ��ȡHeader�еķָ���
	*/
	Split* GetHeaderSplit() const { return m_pHeaderSplit; }

public:
	/** ����һ������
	* @param [in] groupName ���������
	* @param [in] description �����������Ϣ
	* @param [in] nGroupData �û��Զ�������
	* @return ���ظ÷���Ľӿڣ���������������
	*/
	PropertyGridGroup* AddGroup(const std::wstring& groupName, 
							    const std::wstring& description = L"",
								size_t nGroupData = 0);

	/** ��ȡ���еķ���
	* @param [out] groups ���ص�ǰ���еķ����б�
	*/
	void GetGroups(std::vector<PropertyGridGroup*>& groups) const;

	/** ɾ������
	* @param [in] pGroup ��ɾ���ķ���
	*/
	bool RemoveGroup(PropertyGridGroup* pGroup);

	/** ɾ�����з���
	*/
	void RemoveAllGroups();

	/** ����һ������
	* @param [in] pGroup �����������ķ���
	* @param [in] propertyName ���Ե�����
	* @param [in] propertyValue ���Ե�ֵ
	* @param [in] description ���Ե�������Ϣ
	* @param [in] nPropertyData �û��Զ�������
	* @return ���ظ����ԵĽӿ�
	*/
	PropertyGridProperty* AddProperty(PropertyGridGroup* pGroup,
									  const std::wstring& propertyName, 
									  const std::wstring& propertyValue,
									  const std::wstring& description = L"",
									  size_t nPropertyData = 0);

	/** �������һ�еĿ���
	* @param [in] nLeftColumnWidth ���һ�еĿ���
    * @param [in] bNeedDpiScale �Ƿ���Ҫ���п�ֵ����DPI����Ӧ
	*/
	void SetLeftColumnWidth(int32_t nLeftColumnWidth, bool bNeedDpiScale);

	/** ��ȡ���һ�еĿ���ֵ
	*/
	int32_t GetLeftColumnWidth() const;

public:
	/** �����Ƿ���ʾ��������
	*/
	void SetEnableDescriptionArea(bool bEnable);

	/** �ж��Ƿ���ʾ��������
	*/
	bool IsEnableDescriptionArea() const { return m_bDescriptionArea; }

	/** ������������ĸ߶�ֵ
	* @param [in] nHeight �߶�ֵ
    * @param [in] bNeedDpiScale �Ƿ���Ҫ���п�ֵ����DPI����Ӧ
	*/
	void SetDescriptionAreaHeight(int32_t nHeight, bool bNeedDpiScale);

	/** ��ȡ��ǰ��������ĸ߶�ֵ
	*/
	int32_t GetDescriptionAreaHeight() const;

	/** ��ȡ�����ؼ��Ľӿ�
	*/
	RichText* GetDescriptionArea() const { return m_pDescriptionArea; }

	/** �����ؼ��ķָ����ӿ�
	*/
	Split* GetDescriptionAreaSplit() const { return m_pDescriptionAreaSplit; }

public:
	/** ��ȡ���Ա������ؼ��ӿڣ����ڹ������ݣ�
	*/
	TreeView* GetTreeView() const { return m_pTreeView; }

	/** ���������ߵĿ���
	* @param [in] nLineWidth �����ߵĿ��ȣ����Ϊ0��ʾ����ʾ����������
	* @param [in] bNeedDpiScale ���Ϊtrue��ʾ��Ҫ�Կ��Ƚ���DPI����Ӧ
	*/
	void SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
	int32_t GetRowGridLineWidth() const;

	/** ���������ߵ���ɫ
	* @param [in] color ���������ߵ���ɫ
	*/
	void SetRowGridLineColor(const std::wstring& color);
	std::wstring GetRowGridLineColor() const;

	/** ���������ߵĿ���
	* @param [in] nLineWidth �����ߵĿ��ȣ����Ϊ0��ʾ����ʾ����������
	* @param [in] bNeedDpiScale ���Ϊtrue��ʾ��Ҫ�Կ��Ƚ���DPI����Ӧ
	*/
	void SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
	int32_t GetColumnGridLineWidth() const;

	/** ���������ߵ���ɫ
	* @param [in] color ���������ߵ���ɫ
	*/
	void SetColumnGridLineColor(const std::wstring& color);
	std::wstring GetColumnGridLineColor() const;

	/** ��ͷ��Class
	*/
	void SetHeaderClass(const std::wstring& headerClass);
	std::wstring GetHeaderClass() const;

	/** �����Class
	*/
	void SetGroupClass(const std::wstring& groupClass);
	std::wstring GetGroupClass() const;

	/** ���Ե�Class
	*/
	void SetPropertyClass(const std::wstring& propertyClass);
	std::wstring GetPropertyClass() const;

protected:
	/** ��ʼ������
	 */
	virtual void DoInit() override;

	/** �����ӿؼ�
	*/
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

	/** ����������
	*/
	void PaintGridLines(IRender* pRender);

	/** �϶��б�ͷ�ı��п����¼���Ӧ����
	*/
	void OnHeaderColumnResized();

	/** ����һ�����Խڵ���п�
	*/
	void ResizePropertyColumn(TreeNode* pPropertyNode, int32_t nLeftColumnWidth);

private:
	/** �Ƿ��Ѿ���ɳ�ʼ��
	*/
	bool m_bInited;

	/** ����XML�ļ�
	*/
	UiString m_configXml;

private:
	/** �Ƿ���ʾHeader
	*/
	bool m_bHeaderCtrl;

	/** Header�ӿ�
	*/
	Control* m_pHeaderCtrl;

	/** Header�е����һ��
	*/
	Label* m_pHeaderLeft;

	/** Header�е��Ҳ�һ��
	*/
	Label* m_pHeaderRight;

	/** Header�еķָ���
	*/
	Split* m_pHeaderSplit;

	/** ���һ�еĿ���
	*/
	int32_t m_nLeftColumnWidth;

private:
	/** �����ؼ��ķָ���
	*/
	Split* m_pDescriptionAreaSplit;

	/** �����ؼ�
	*/
	RichText* m_pDescriptionArea;

	/** �Ƿ���ʾ��������
	*/
	bool m_bDescriptionArea;

private:
	/** ���Ա������ؼ��ӿ�
	*/
	TreeView* m_pTreeView;

	/** ��ͷ��Class
	*/
	UiString m_headerClass;

	/** �����Class
	*/
	UiString m_groupClass;

	/** ���Ե�Class
	*/
	UiString m_propertyClass;

private:
	/** ���������ߵĿ���
	*/
	int32_t m_nRowGridLineWidth;

	/** ���������ߵ���ɫ
	*/
	UiString m_rowGridLineColor;

	/** ���������ߵĿ���
	*/
	int32_t m_nColumnGridLineWidth;

	/** ���������ߵ���ɫ
	*/
	UiString m_columnGridLineColor;
};

/** ���Ա��ķ���, �����ṹ
*   <PropertyGridGroup>
*		<HBox>
*			<LabelBox/>
*		</HBox>
*   </PropertyGridGroup>
*/
class PropertyGridGroup : public TreeNode
{
public:
	/** ����һ����
	@param [in] groupName �������
	@param [in] description ���������Ϣ
	@param [in] nGroupData �û��Զ�������
	*/
	explicit PropertyGridGroup(const std::wstring& groupName, 
							   const std::wstring& description = L"",
		                       size_t nGroupData = 0);

public:
	/** ��ȡ��������
	*/
	std::wstring GetGroupName() const { return m_groupName.c_str(); }

	/** ��ȡ���������Ϣ
	*/
	std::wstring GetDescriptiion() const { return m_description.c_str(); }

	/** ��ȡ�û��Զ�������
	*/
	size_t GetGroupData() const { return m_nGroupData; }

	/** �����û��Զ�������
	*/
	void SetGroupData(size_t nGroupData) { m_nGroupData = nGroupData; }

	/** ��ȡ�������Ƶ���ʾ�ؼ�
	*/
	LabelBox* GetLabelBox() const { return m_pLabelBox; }

	/** ��ȡ�����µ���������
	* @param [out] properties ���ص�ǰ���еķ����б�
	*/
	void GetProperties(std::vector<PropertyGridProperty*>& properties) const;

	/** ɾ���÷����µ�����
	* @param [in] pProperty ��Ҫɾ�������Խӿ�
	*/
	bool RemoveProperty(PropertyGridProperty* pProperty);

	/** ɾ���÷����µ���������
	*/
	void RemoveAllProperties();

protected:
	/** ��ʼ������
	 */
	virtual void DoInit() override;

private:
	/** �Ƿ��Ѿ���ɳ�ʼ��
	*/
	bool m_bInited;

	/** ���������
	*/
	UiString m_groupName;

	/** ���������Ϣ
	*/
	UiString m_description;

	/** �û��Զ�������
	*/
	size_t m_nGroupData;

	/** �������Ƶ���ʾ�ؼ�
	*/
	LabelBox* m_pLabelBox;
};

/** ���Ա�������, �����ṹ
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
	/** ����һ������
	@param [in] propertyName ���Ե�����
	@param [in] propertyValue ���Ե�ֵ
	@param [in] description ���Ե�������Ϣ
	@param [in] nPropertyData �û��Զ�������
	*/
	PropertyGridProperty(const std::wstring& propertyName, 
					     const std::wstring& propertyValue,
					     const std::wstring& description = L"",
					     size_t nPropertyData = 0);

public:
	/** ��ȡ��������
	*/
	std::wstring GetPropertyName() const { return m_propertyName.c_str(); }

	/** ��ȡ����ֵ
	*/
	std::wstring GetPropertyValue() const { return m_propertyValue.c_str(); }

	/** ��ȡ�µ�����ֵ���޸ĺ������ֵ, ������޸��򷵻�ԭֵ��
	*/
	std::wstring GetNewPropertyValue() const;

	/** ��ȡ���Ե�������Ϣ
	*/
	std::wstring GetDescriptiion() const { return m_description.c_str(); }

	/** ��ȡ�û��Զ�������
	*/
	size_t GetPropertyData() const { return m_nPropertyData; }

	/** �����û��Զ�������
	*/
	void SetPropertyData(size_t nPropertyData) { m_nPropertyData = nPropertyData; }

	/** ��ȡ�������ƺ�����ֵ���������ؼ������������ñ���ɫ��
	*/
	HBox* GetHBox() const {	return m_pHBox;	}

	/** ��ȡ�������Ƶ���ʾ�ؼ�, ���ؼ���GetHBox()
	*/
	LabelBox* GetLabelBoxLeft() const { return m_pLabelBoxLeft; }

	/** ��ȡ����ֵ����ʾ�ؼ�, ���ؼ���GetHBox()
	*/
	LabelBox* GetLabelBoxRight() const { return m_pLabelBoxRight; }

	/** ��ȡ�༭��ؼ�
	*/
	RichEdit* GetRichEdit() const { return m_pRichEdit; }

	/** ����ֻ��ģʽ
	*/
	void SetReadOnly(bool bReadOnly);

	/** �Ƿ�Ϊֻ��ģʽ
	*/
	bool IsReadOnly() const { return m_bReadOnly; }

	/** ��������ģʽ����ʾ ***��
	 * @param[in] bPassword ����Ϊ true �ÿؼ���ʾ����Ϊ ***��false Ϊ��ʾ��������
	 */
	void SetPassword(bool bPassword);

	/** �Ƿ�Ϊ����ģʽ
	*/
	bool IsPassword() const { return m_bPassword; }

	/** �����Ƿ�֧��Spin�ؼ�
	* @param [in] bEnable true��ʾ֧��Spin�ؼ���false��ʾ��֧��Spin�ؼ�
	* @param [in] nMin ��ʾ�������ֵ���Сֵ
	* @param [in] nMax ��ʾ�������ֵ����ֵ����� nMin��nMaxͬʱΪ0, ��ʾ���������ֵ���Сֵ�����ֵ
	*/
	void SetEnableSpin(bool bEnable, int32_t nMin = 0, int32_t nMax = 0);

protected:
	/** ��ʼ������
	 */
	virtual void DoInit() override;

	/** �����Ƿ���ʾ�༭��ؼ�
	* @param [in] bShow true��ʾ��ʾ�༭��ؼ���false��ʾ����ʾ�༭��ؼ�
	*/
	void SetShowRichEdit(bool bShow);

private:
	/** �Ƿ��Ѿ���ɳ�ʼ��
	*/
	bool m_bInited;

	/** ���Ե�����
	*/
	UiString m_propertyName;

	/** ���Ե�ֵ
	*/
	UiString m_propertyValue;

	/** ���Ե�������Ϣ
	*/
	UiString m_description;

	/** �û��Զ�������
	*/
	size_t m_nPropertyData;

	/** �������ƺ�����ֵ���������ؼ�
	*/
	HBox* m_pHBox;

	/** �������Ƶ���ʾ�ؼ�
	*/
	LabelBox* m_pLabelBoxLeft;

	/** ����ֵ����ʾ�ؼ�
	*/
	LabelBox* m_pLabelBoxRight;

	/** �༭��ؼ�(�����޸�����)
	*/
	RichEdit* m_pRichEdit;

	/** ֻ��ģʽ
	*/
	bool m_bReadOnly;

	/** ����ģʽ
	*/
	bool m_bPassword;
};

}//namespace ui

#endif //UI_CONTROL_PROPERTY_GRID_H_