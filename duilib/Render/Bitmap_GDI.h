#ifndef UI_RENDER_BITMAP_GDI_H_
#define UI_RENDER_BITMAP_GDI_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui
{

/** λͼ��ʵ�֣�GDI��������
*/
class UILIB_API Bitmap_GDI: public IBitmap
{
public:
	Bitmap_GDI();
	Bitmap_GDI(HBITMAP hBitmap, bool flipHeight);
	virtual ~Bitmap_GDI();

public:
	/** �����ݳ�ʼ����ARGB��ʽ��
	@param [in] nWidth ����
	@param [in] nHeight �߶�
	@param [in] flipHeight �Ƿ�תλͼ�����Ϊtrue������λͼ��ʱ�������Ͻ�ΪԲ�㣬ͼ�����Ǵ��ϵ��µģ�
						   ���Ϊfalse���������½�ΪԲ�㣬ͼ�����Ǵ��µ��ϡ�
	@param [in] pPixelBits λͼ����, ���Ϊnullptr��ʾ���ڿ�λͼ�������Ϊnullptr�������ݳ���Ϊ��nWidth*4*nHeight
	*/
	virtual bool Init(uint32_t nWidth, uint32_t nHeight, bool flipHeight, const void* pPixelBits) override;

	/** ��ȡͼƬ����
	*/
	virtual uint32_t GetWidth() const override;

	/** ��ȡͼƬ�߶�
	*/
	virtual uint32_t GetHeight() const override;

	/** ��ȡͼƬ��С
	@return ͼƬ��С
	*/
	virtual UiSize GetSize() const override;

	/** ����λͼ���ݣ����ݳ��� = GetWidth() * GetHeight() * 4
	*/
	virtual void* LockPixelBits() override;

	/** �ͷ�λͼ����
	*/
	virtual void UnLockPixelBits() override;

	/** ��λͼ�Ƿ���͸������(��͸��ͨ���У����в���255������)
	*/
	virtual bool IsAlphaBitmap() const override;

	/** ��¡�����µĵ�λͼ
	*@return ���������ɵ�λͼ�ӿڣ��ɵ��÷��ͷ���Դ
	*/
	virtual IBitmap* Clone() override;

public:
	/** ��ȡλͼGDI���
	*/
	HBITMAP GetHBitmap() const;

	/* �����λͼ����Ĺ���
	*/
	HBITMAP DetachHBitmap();

	/** ����һ���豸�޹ص�λͼ
	*@return ����λͼ������ɵ��÷��ͷ�λͼ��Դ
	*/
	static HBITMAP CreateBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits);

private:
	/** ����ͼƬ��͸��ͨ����־
	*/
	void UpdateAlphaFlag(const uint8_t* pPixelBits);

private:
	//λͼGDI���
	HBITMAP m_hBitmap;

	//λͼ�Ŀ���
	uint32_t m_nWidth;
	
	//λͼ�Ŀ���
	uint32_t m_nHeight;

	/**λͼ�����־Ϊ��true��ʾλͼ���򣺴��ϵ��£����Ͻ�ΪԲ��
	                 false��ʾ��λͼ���򣺴��µ��ϣ����½�ΪԲ��
	*/
	bool m_bFlipHeight;

	/** ��λͼ�Ƿ���͸������(��͸��ͨ���У����в���255������)
	*/
	bool m_bAlphaBitmap;
};

} // namespace ui

#endif // UI_RENDER_BITMAP_GDI_H_