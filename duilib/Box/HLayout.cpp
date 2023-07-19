#include "HLayout.h"

namespace ui 
{

HLayout::HLayout()
{
}

UiSize64 HLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize szAvailable(rc.Width(), rc.Height());
	szAvailable.Validate();
	if (rc.Width() < 0) {
		rc.right = rc.left;
	}
	if (rc.Height() < 0) {
		rc.bottom = rc.top;
	}

	//����Ϊstretch�Ŀؼ���
	int32_t stretchCount = 0;
	//�̶����ȵĿؼ����ܵĸ߶�
	int32_t cxFixedTotal = 0;
	//��Ҫ���в��ִ��������пؼ�(KEY�ǿؼ���VALUE�ǿ��Ⱥ͸߶�)
	std::map<Control*, UiSize> itemsMap;

	//����ÿ���ؼ��Ŀ��Ⱥ͸߶ȣ�����¼��Map��
	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}

		UiSize sz = pControl->EstimateSize(szAvailable);
		ASSERT((sz.cx >= DUI_LENGTH_STRETCH) && (sz.cy >= DUI_LENGTH_STRETCH));

		UiRect rcMargin = pControl->GetMargin();
		//�������
		if( sz.cx == DUI_LENGTH_STRETCH ) {
			stretchCount++;
			cxFixedTotal += (rcMargin.left + rcMargin.right);
		}
		else {
			if (sz.cx < pControl->GetMinWidth()) {
				sz.cx = pControl->GetMinWidth();
			}
			if (sz.cx > pControl->GetMaxWidth()) {
				sz.cx = pControl->GetMaxWidth();
			}
			if (sz.cx < 0) {
				sz.cx = 0;
			}
			cxFixedTotal += (sz.cx + rcMargin.left + rcMargin.right);
		}
		
		//����߶�
		if (sz.cy == DUI_LENGTH_STRETCH) {
			sz.cy = (szAvailable.cy - rcMargin.top - rcMargin.bottom);
		}
		if (sz.cy < pControl->GetMinHeight()) {
			sz.cy = pControl->GetMinHeight();
		}
		if (sz.cy > pControl->GetMaxHeight()) {
			sz.cy = pControl->GetMaxHeight();
		}
		if (sz.cy < 0) {
			sz.cy = 0;
		}
		itemsMap[pControl] = sz;
	}
	if (!itemsMap.empty()) {
		cxFixedTotal += ((int32_t)itemsMap.size() - 1) * GetChildMarginX();
	}

	int32_t cxStretch = 0;
	if (stretchCount > 0) {
		cxStretch = std::max(0, (szAvailable.cx - cxFixedTotal) / stretchCount);
	}
	//��һ��Ԥ����ȥ����Ҫʹ��minwidth/maxwidth�Ŀؼ��������¼���ƽ���߶�
	if ((cxStretch > 0) && !itemsMap.empty()) {
		for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter) {
			Control* pControl = iter->first;
			UiSize sz = iter->second;
			if (sz.cx == DUI_LENGTH_STRETCH) {
				sz.cx = cxStretch;
				if (sz.cx < pControl->GetMinWidth()) {
					sz.cx = pControl->GetMinWidth();
				}
				if (sz.cx > pControl->GetMaxWidth()) {
					sz.cx = pControl->GetMaxWidth();
				}
				if (sz.cx != cxStretch) {
					//����ؼ���Ҫʹ��min����max���ȣ���ƽ��ֵ���Ƴ�������Fixed�ؼ���
					iter->second = sz;
					--stretchCount;
					cxFixedTotal += sz.cx; //Margin�Ѿ��ۼӹ�������Ҫ�����ۼ�
				}
			}
		}
	}
	//���¼���Stretch�ؼ��Ŀ��ȣ���������μ����Ϊ׼��
	//��������ܿռ䲻�㣬��ԭ��������ƽ���߶ȣ����ȱ�֤ǰ��Ŀؼ�����������ʾ
	if ((stretchCount > 0) && (szAvailable.cx > cxFixedTotal)) {
		cxStretch = std::max(0, (szAvailable.cx - cxFixedTotal) / stretchCount);
	}

	// Position the elements
	int32_t deviation = szAvailable.cx - cxFixedTotal - cxStretch * stretchCount;//ʣ����ÿ��ȣ����ھ���ƫ��
	if (deviation < 0) {
		deviation = 0;
	}

	int32_t iPosTop = rc.top;
	int32_t iPosBottom = rc.bottom;
	int32_t iPosX = rc.left;

	// Place elements
	int64_t cyNeeded = 0;//��Ҫ���ܸ߶ȣ�ȡ�����ӿؼ��ĸ߶����ֵ��
	int64_t cxNeeded = 0;//��Ҫ���ܿ���

	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if (pControl->IsFloat()) {
			SetFloatPos(pControl, rc);
			continue;
		}

		UiRect rcMargin = pControl->GetMargin();
		ASSERT(itemsMap.find(pControl) != itemsMap.end());
		UiSize sz = itemsMap[pControl];

		//�������
		if( sz.cx == DUI_LENGTH_STRETCH ) {
			sz.cx = cxStretch;
			if (sz.cx < pControl->GetMinWidth()) {
				sz.cx = pControl->GetMinWidth();
			}
			if (sz.cx > pControl->GetMaxWidth()) {
				sz.cx = pControl->GetMaxWidth();
			}
			if ((sz.cx <= cxStretch) && (deviation > 0)) {
				sz.cx += 1;
				deviation--;
			}
		}

		//����������뷽ʽ��ȷ��Y������
		int32_t childTop = 0;
		int32_t childBottm = 0;
		VerAlignType verAlignType = pControl->GetVerAlignType();
		if (verAlignType == kVerAlignTop) {
			childTop = iPosTop + rcMargin.top;
			childBottm = childTop + sz.cy;
		}
		else if (verAlignType == kVerAlignBottom) {
			childBottm = iPosBottom - rcMargin.bottom;
			childTop = childBottm - sz.cy;
		}
		else if (verAlignType == kVerAlignCenter) {
			childTop = iPosTop + (iPosBottom - iPosTop + rcMargin.top - rcMargin.bottom - sz.cy) / 2;
			childBottm = childTop + sz.cy;
		}

		UiRect rcChildPos(iPosX + rcMargin.left, childTop, iPosX + rcMargin.left + sz.cx, childBottm);
		pControl->SetPos(rcChildPos);
		cyNeeded = std::max(cyNeeded, (int64_t)rcChildPos.Height());

		iPosX += (sz.cx + rcMargin.left + GetChildMarginX() + rcMargin.right);
		cxNeeded += (sz.cx + rcMargin.left + rcMargin.right);
	}
	if (!itemsMap.empty()) {
		cxNeeded += ((int64_t)itemsMap.size() - 1) * GetChildMarginX();
	}

	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize HLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize totalSize;
	UiSize itemSize;
	int32_t estimateCount = 0;
	for(Control* pControl : items)	{
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()){
			continue;
		}

		estimateCount++;
		UiRect rcMargin = pControl->GetMargin();
		itemSize = pControl->EstimateSize(szAvailable);
		if (itemSize.cx < pControl->GetMinWidth()) {
			itemSize.cx = pControl->GetMinWidth();
		}
		if (itemSize.cx > pControl->GetMaxWidth()) {
			itemSize.cx = pControl->GetMaxWidth();
		}
		if (itemSize.cy < pControl->GetMinHeight()) {
			itemSize.cy = pControl->GetMinHeight();
		}
		if (itemSize.cy > pControl->GetMaxHeight()) {
			itemSize.cy = pControl->GetMaxHeight();
		}
		
		totalSize.cy = std::max(itemSize.cy + rcMargin.top + rcMargin.bottom, totalSize.cy);
		totalSize.cx += (itemSize.cx + rcMargin.left + rcMargin.right);
	}

	if ((estimateCount - 1) > 0) {
		totalSize.cx += (estimateCount - 1) * GetChildMarginX();
	}

	UiRect rcPadding = GetPadding();
	totalSize.cx += (rcPadding.left + rcPadding.right);
	totalSize.cy += (rcPadding.top + rcPadding.bottom);
	return totalSize;
}

} // namespace ui