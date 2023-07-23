#include "Layout.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

namespace ui 
{
Layout::Layout() :
	m_rcPadding(0, 0, 0, 0),
	m_iChildMarginX(0),
	m_iChildMarginY(0),
	m_pOwner(nullptr)
{
}

void Layout::SetOwner(Box* pOwner)
{
	m_pOwner = pOwner;
}

UiSize64 Layout::SetFloatPos(Control* pControl, const UiRect& rcContainer)
{
	ASSERT(pControl != nullptr);
	if ((pControl == nullptr) || (!pControl->IsVisible())) {
		return UiSize64();
	}
	UiMargin rcMargin = pControl->GetMargin();
	UiRect rc = rcContainer;
	rc.Deflate(rcMargin);
	UiSize szAvailable(rc.Width(), rc.Height());
	szAvailable.Validate();
	UiEstSize estSize = pControl->EstimateSize(szAvailable);
	UiSize childSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
	if (estSize.cx.IsStretch()) {
		childSize.cx = szAvailable.cx;		
	}
	if (estSize.cy.IsStretch()) {
		childSize.cy = szAvailable.cy;		
	}
	childSize.cx = std::max(childSize.cx, 0);
	childSize.cy = std::max(childSize.cy, 0);
	
	if (childSize.cx < pControl->GetMinWidth()) {
		childSize.cx = pControl->GetMinWidth();
	}
	if (childSize.cx > pControl->GetMaxWidth()) {
		childSize.cx = pControl->GetMaxWidth();
	}
	
	if (childSize.cy < pControl->GetMinHeight()) {
		childSize.cy = pControl->GetMinHeight();
	}
	if (childSize.cy > pControl->GetMaxHeight()) {
		childSize.cy = pControl->GetMaxHeight();
	}
	UiRect childPos = GetFloatPos(pControl, rcContainer, childSize);
	pControl->SetPos(childPos);
	//TODO: 64位兼容性检查
	return UiSize64(childPos.Width(), childPos.Height());
}

UiRect Layout::GetFloatPos(Control* pControl, UiRect rcContainer, UiSize childSize)
{
	if (rcContainer.Width() < 0) {
		rcContainer.right = rcContainer.left;
	}
	if (rcContainer.Height() < 0) {
		rcContainer.bottom = rcContainer.top;
	}

	UiMargin rcMargin = pControl->GetMargin();
	int32_t iPosLeft = rcContainer.left + rcMargin.left;
	int32_t iPosRight = rcContainer.right - rcMargin.right;
	int32_t iPosTop = rcContainer.top + rcMargin.top;
	int32_t iPosBottom = rcContainer.bottom - rcMargin.bottom;
	//如果空间不足，则宽高设置为零（如果界面可用调整大小，这个情况会频繁出现）
	if (iPosRight < iPosLeft) {
		iPosRight = iPosLeft;
	}
	if (iPosBottom < iPosTop) {
		iPosBottom = iPosTop;
	}
	
	childSize.cx = std::max(childSize.cx, 0);
	childSize.cy = std::max(childSize.cy, 0);

	int32_t childWidth = childSize.cx;
	int32_t childHeight = childSize.cy;

	//按照子控件指定的横向对齐方式和纵向对齐方式来排列控件
	HorAlignType horAlignType = pControl->GetHorAlignType();
	VerAlignType verAlignType = pControl->GetVerAlignType();

	int32_t childLeft = 0;
	int32_t childRight = 0;
	int32_t childTop = 0;
	int32_t childBottm = 0;

	if (horAlignType == kHorAlignLeft) {
		childLeft = iPosLeft;
		childRight = childLeft + childWidth;
	}
	else if (horAlignType == kHorAlignRight) {
		childRight = iPosRight;
		childLeft = childRight - childWidth;
	}
	else if (horAlignType == kHorAlignCenter) {
		childLeft = iPosLeft + (iPosRight - iPosLeft - childWidth) / 2;
		childRight = childLeft + childWidth;
	}

	if (verAlignType == kVerAlignTop) {
		childTop = iPosTop;
		childBottm = childTop + childHeight;
	}
	else if (verAlignType == kVerAlignBottom) {
		childBottm = iPosBottom;
		childTop = childBottm - childHeight;
	}
	else if (verAlignType == kVerAlignCenter) {
		childTop = iPosTop + (iPosBottom - iPosTop - childHeight) / 2;
		childBottm = childTop + childHeight;
	}

	UiRect childPos(childLeft, childTop, childRight, childBottm);
	return childPos;
}

bool Layout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	bool hasAttribute = true;
	if (strName == L"padding") {
		UiPadding rcPadding;
		AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
		SetPadding(rcPadding, true);
	}
	else if ((strName == L"child_margin") || (strName == L"childmargin")) {
		int32_t iMargin = _wtoi(strValue.c_str());
		SetChildMargin(iMargin);
	}
	else if ((strName == L"child_margin_x") || (strName == L"childmarginx")) {
		int32_t iMargin = _wtoi(strValue.c_str());
		SetChildMarginX(iMargin);
	}
	else if ((strName == L"child_margin_y") || (strName == L"childmarginy")) {
		int32_t iMargin = _wtoi(strValue.c_str());
		SetChildMarginY(iMargin);
	}
	else {
		hasAttribute = false;
	}

	return hasAttribute;
}

UiSize64 Layout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	DeflatePadding(rc);
	UiSize64 size;
	for (Control* pControl : items) {
		if ((pControl == nullptr) || (!pControl->IsVisible())) {
			continue;
		}
		UiSize64 controlSize = SetFloatPos(pControl, rc);
		size.cx = std::max(size.cx, controlSize.cx);
		size.cy = std::max(size.cy, controlSize.cy);
	}
	return size;
}

UiSize Layout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	//宽度：取所有子控件宽度的最大值，加上Margin、Padding等，不含拉伸类型的子控件
	//高度：取所有子控件高度的最大值，加上Margin、Padding等，不含拉伸类型的子控件
	szAvailable.Validate();
	UiSize maxSize;
	UiSize itemSize;
	for (Control* pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}
		UiEstSize estSize = pControl->EstimateSize(szAvailable);
		itemSize = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
		if (estSize.cx.IsStretch()) {
			//拉伸类型的子控件，不计入
			itemSize.cx = 0;
		}
		else {
			if (itemSize.cx < pControl->GetMinWidth()) {
				itemSize.cx = pControl->GetMinWidth();
			}
			if (itemSize.cx > pControl->GetMaxWidth()) {
				itemSize.cx = pControl->GetMaxWidth();
			}
		}
		if (estSize.cy.IsStretch()) {
			//拉伸类型的子控件，不计入
			itemSize.cy = 0;
		}
		else {
			if (itemSize.cy < pControl->GetMinHeight()) {
				itemSize.cy = pControl->GetMinHeight();
			}
			if (itemSize.cy > pControl->GetMaxHeight()) {
				itemSize.cy = pControl->GetMaxHeight();
			}
		}
		UiMargin rcMargin = pControl->GetMargin();
		if (itemSize.cx > 0) {
			maxSize.cx = std::max(itemSize.cx + rcMargin.left + rcMargin.right, maxSize.cx);
		}
		if (itemSize.cy > 0) {
			maxSize.cy = std::max(itemSize.cy + rcMargin.top + rcMargin.bottom, maxSize.cy);
		}
	}
	if (maxSize.cx > 0) {
		maxSize.cx += m_rcPadding.left + m_rcPadding.right;
	}
	if (maxSize.cy > 0) {
		maxSize.cy += m_rcPadding.top + m_rcPadding.bottom;
	}
	if ((maxSize.cx == 0) || (maxSize.cy == 0)){
		CheckConfig(items);
	}
	return maxSize;
}

void Layout::CheckConfig(const std::vector<Control*>& items)
{
	//如果m_pOwner的宽高都是auto，而且子控件的宽高都是stretch，那么得到的结果是零，增加个断言
	if (m_pOwner == nullptr) {
		return;
	}
	if (!m_pOwner->GetFixedWidth().IsAuto() && !m_pOwner->GetFixedHeight().IsAuto()) {
		return;
	}

	bool isAllWidthStretch = true;
	bool isAllHeightStretch = true;
	size_t childCount = 0;
	for (Control* pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}
		if (!pControl->GetFixedWidth().IsStretch()) {
			isAllWidthStretch = false;
		}
		if (!pControl->GetFixedHeight().IsStretch()) {
			isAllHeightStretch = false;
		}
		++childCount;
	}
	if ((childCount > 0) && m_pOwner->GetFixedWidth().IsAuto() && isAllWidthStretch) {
		ASSERT(!"配置错误：当前容器的宽是auto，子控件的宽都是stretch，估算宽度为零！");
	}
	if ((childCount > 0) && m_pOwner->GetFixedHeight().IsAuto() && isAllHeightStretch) {
		ASSERT(!"配置错误：当前容器的高是auto，子控件的高都是stretch，估算高度为零！");
	}
}

void Layout::SetPadding(UiPadding rcPadding, bool bNeedDpiScale /*= true*/)
{
	ASSERT((rcPadding.left >= 0) && (rcPadding.top >= 0) && (rcPadding.right >= 0) && (rcPadding.bottom >= 0));
	rcPadding.Validate();
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScalePadding(rcPadding);
	}
	m_rcPadding = rcPadding;
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner != nullptr) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMargin(int32_t iMargin)
{
	ASSERT(iMargin >= 0);
	iMargin = std::max(iMargin, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMargin);
	bool isChanged = (m_iChildMarginX != iMargin) || (m_iChildMarginY != iMargin);
	m_iChildMarginX = iMargin;
	m_iChildMarginY = iMargin;
	ASSERT(m_pOwner != nullptr);
	if (isChanged && (m_pOwner != nullptr)) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMarginX(int32_t iMarginX)
{
	ASSERT(iMarginX >= 0);
	iMarginX = std::max(iMarginX, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMarginX);
	bool isChanged = (m_iChildMarginX != iMarginX);
	m_iChildMarginX = iMarginX;
	ASSERT(m_pOwner != nullptr);
	if (isChanged && (m_pOwner != nullptr)) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMarginY(int32_t iMarginY)
{
	ASSERT(iMarginY >= 0);
	iMarginY = std::max(iMarginY, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMarginY);
	bool isChanged = (m_iChildMarginY != iMarginY);
	m_iChildMarginY = iMarginY;
	ASSERT(m_pOwner != nullptr);
	if (isChanged && (m_pOwner != nullptr)) {
		m_pOwner->Arrange();
	}
}

UiRect Layout::GetInternalPos() const
{
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner == nullptr) {
		return UiRect();
	}
	UiRect internalPos = m_pOwner->GetPos();
	internalPos.Deflate(m_rcPadding);
	return internalPos;
}

void Layout::DeflatePadding(UiRect& rc) const
{
	rc.Deflate(m_rcPadding);
	if (rc.Width() < 0) {
		rc.right = rc.left;
	}
	if (rc.Height() < 0) {
		rc.bottom = rc.top;
	}
}

} // namespace ui
