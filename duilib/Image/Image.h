#ifndef UI_CORE_IMAGEDECODE_H_
#define UI_CORE_IMAGEDECODE_H_

#pragma once

#include "duilib/Core/Define.h"
#include "duilib/duilib_defs.h"
#include "duilib/Render/IRender.h"
#include <memory>
#include <string>
#include <map>
#include <vector>

namespace ui 
{
	class IRender;

/** 图片信息
*/
class UILIB_API ImageInfo
{
public:
	ImageInfo();
	~ImageInfo();

	ImageInfo(const ImageInfo&) = delete;
	ImageInfo& operator = (const ImageInfo&) = delete;

public:
	/** 设置图片路径
	*/
	void SetImageFullPath(const std::wstring& path);

	/** 获取图片路径
	*/
	const std::wstring& GetImageFullPath() const;

	/** 设置该图片的大小是否已经做过适应DPI处理
	*/
	void SetBitmapSizeDpiScaled(bool isDpiScaled) { m_bDpiScaled = isDpiScaled; }

	/** 判断该图片的大小是否已经做过适应DPI处理
	*/
	bool IsBitmapSizeDpiScaled() const { return m_bDpiScaled; }

	/** 添加一个图片帧数据, 添加后该资源由该类内部托管
	*/
	void PushBackHBitmap(IBitmap* pBitmap);

	/** 获取一个图片帧数据
	*/
	IBitmap* GetBitmap(size_t nIndex) const;

	/** 设置图片的宽和高
	*/
	void SetImageSize(int nWidth, int nHeight);

	/** 获取图片宽度
	*/
	int GetWidth() const { return m_nWidth; }

	/** 获取图片高度
	*/
	int GetHeight() const { return m_nHeight; }

	/** 获取图片的帧数
	*/
	size_t GetFrameCount() const;

	/** 是否位多帧图片(比如GIF等)
	*/
	bool IsMultiFrameImage() const;

	/** 设置图片的多帧播放事件间隔（毫秒为单位 ）
	*/
	void SetFrameInterval(const std::vector<int>& frameIntervals);

	/** 获取图片帧对应的播放时间间隔（毫秒为单位 ）
	*/
	int GetFrameInterval(size_t nIndex);

	/** 设置循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
	*/
	void SetPlayCount(int32_t nPlayCount);

	/** 获取循环播放次数
	*@return 返回值：-1 表示未设置
	*               0  表示动画是一致循环播放的
	*              > 0 表示动画循环播放的具体次数
	*/
	int32_t GetPlayCount() const;

	/** 设置图片的缓存KEY, 用于图片的生命周期管理
	*/
	void SetCacheKey(const std::wstring& cacheKey);

	/** 获取图片的缓存KEY
	*/
	const std::wstring& GetCacheKey() const;

private:
	//图片的完整路径
	std::wstring m_imageFullPath;

	//该图片的大小是否已经做过适应DPI处理
	bool m_bDpiScaled;

	//图片的宽度
	int m_nWidth;
	
	//图片的高度
	int m_nHeight;

	//图片帧对应的播放时间间隔（毫秒为单位 ）
	std::vector<int> m_frameIntervals;

	//图片帧数据
	std::vector<IBitmap*> m_frameBitmaps;

	//循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
	int32_t m_nPlayCount;

	/** 图片的缓存KEY, 用于图片的生命周期管理
	*/
	std::wstring m_cacheKey;
};

/** 图片属性
*/
class UILIB_API ImageAttribute
{
public:
	ImageAttribute();

	/** 对数据成员进行初始化
	*/
	void Init();

	/** 根据图片参数进行初始化(先调用Init初始化成员变量，再按照传入参数进行更新部分属性)
	* @param [in] strImageString 图片参数字符串
	*/
	void InitByImageString(const std::wstring& strImageString);

	/** 根据图片参数修改属性值（仅更新新设置的图片属性, 未包含的属性不进行更新）
	* @param [in] strImageString 图片参数字符串
	*/
	void ModifyAttribute(const std::wstring& strImageString);

public:
	//图片文件属性字符串
	std::wstring sImageString;

	//图片文件文件名，含相对路径，不包含属性
	std::wstring sImagePath;

	//设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
	std::wstring srcWidth;

	//设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
	std::wstring srcHeight;

	//加载图片时，DPI自适应属性，即按照DPI缩放图片大小
	bool srcDpiScale;

	//加载图片时，是否设置了DPI自适应属性
	bool bHasSrcDpiScale;

	//绘制目标区域位置和大小（相对于控件区域的位置）
	UiRect rcDest;

	//图片源区域位置和大小
	UiRect rcSource;

	//圆角属性
	UiRect rcCorner;

	//透明度（0 - 255）
	BYTE bFade;

	//横向平铺
	bool bTiledX;

	//横向完全平铺，未用到
	bool bFullTiledX;

	//纵向平铺
	bool bTiledY;

	//横向完全平铺
	bool bFullTiledY;

	//平铺时的边距
	int nTiledMargin;

	//如果是GIF等动画图片，可以指定播放次数 -1 ：一直播放，缺省值。
	int nPlayCount;	

	//如果是ICO文件，用于指定需要加载的ICO图片的大小
	//(ICO文件中包含很多个不同大小的图片，常见的有256，48，32，16，并且每个大小都有32位真彩、256色、16色之分）
	//目前ICO文件在加载时，只会选择一个大小的ICO图片进行加载，加载后为单张图片
	uint32_t iconSize;
};

/** 图片加载属性，用于加载一个图片
*/
class UILIB_API ImageLoadAttribute
{
public:
	ImageLoadAttribute(const std::wstring& srcWidth,
					   const std::wstring& srcHeight,
		               bool srcDpiScale,
		               bool bHasSrcDpiScale,
		               uint32_t iconSize);

	/** 设置图片路径（本地绝对路径或者压缩包内的相对路径）
	*/
	void SetImageFullPath(const std::wstring& imageFullPath);

	/** 获取图片路径（本地绝对路径或者压缩包内的相对路径）
	*/
	const std::wstring& GetImageFullPath() const;

	/** 获取加载图片的缓存KEY
	*/
	std::wstring GetCacheKey() const;

	/** 设置加载图片时，是否按照DPI缩放图片大小
	*/
	void SetNeedDpiScale(bool bNeedDpiScale);

	/** 获取加载图片时，是否按照DPI缩放图片大小
	*/
	bool NeedDpiScale() const;

	/** 获取加载图片时，是否设置了DPI自适应属性
	*/
	bool HasSrcDpiScale() const;

	/** 获取图片加载后应当缩放的宽度和高度
	* @param [in,out] nImageWidth 传入原始图片的宽度，返回计算后的宽度
	* @param [in,out] nImageHeight 原始图片的高度，返回计算后的高度
	* @return 返回true表示图片大小有缩放，返回false表示图片大小无缩放
	*/
	bool CalcImageLoadSize(uint32_t& nImageWidth, uint32_t& nImageHeight) const;

	/** 如果是ICO文件，用于指定需要加载的ICO图片的大小
	*/
	uint32_t GetIconSize() const;

private:
	/** 获取设置的缩放后的大小
	*/
	uint32_t GetScacledSize(const std::wstring& srcSize, uint32_t nImageSize) const;

private:
	//本地绝对路径或者压缩包内的相对路径，不包含属性
	std::wstring m_srcImageFullPath;

	//设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
	std::wstring m_srcWidth;

	//设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
	std::wstring m_srcHeight;

	//加载图片时，按照DPI缩放图片大小
	bool m_srcDpiScale;

	//加载图片时，是否设置了DPI自适应属性
	bool m_bHasSrcDpiScale;

	//如果是ICO文件，用于指定需要加载的ICO图片的大小
	//(ICO文件中包含很多个不同大小的图片，常见的有256，48，32，16，并且每个大小都有32位真彩、256色、16色之分）
	//目前ICO文件在加载时，只会选择一个大小的ICO图片进行加载，加载后为单张图片
	uint32_t m_iconSize;
};

/** 图片相关封装，支持的文件格式：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
*/
class UILIB_API Image
{
public:
	Image();

	/** 初始化图片属性
	*/
	void InitImageAttribute();

	/** 设置并初始化图片属性
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(const std::wstring& strImageString);

	/** 获取图片属性（含文件名，和图片设置属性等）
	*/
	const std::wstring& GetImageString() const;

	/** 获取图片文件名（含相对路径，不含图片属性）
	*/
	const std::wstring& GetImagePath() const;

	/** 获取图片属性（只读）
	*/
	const ImageAttribute& GetImageAttribute() const;

	/** 获取图片加载属性
	*/
	ImageLoadAttribute GetImageLoadAttribute() const;

public:
	/** 获取图片信息接口
	*/
	const std::shared_ptr<ImageInfo>& GetImageCache() const;

	/** 设置图片信息接口
	*/
	void SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo);

	/** 清除图片信息缓存数据, 释放资源
	*/
	void ClearImageCache();

public:
	/** 设置图片属性：播放次数（仅当多帧图片时）
	*/
	void SetImagePlayCount(int nPlayCount);

	/** 设置图片属性：透明度（仅当多帧图片时）
	*/
	void SetImageFade(uint8_t nFade);

	/** 是否正在播放中（仅当多帧图片时）
	*/
	bool IsPlaying() const { return m_bPlaying; }

	/** 设置是否正在播放中（仅当多帧图片时）
	*/
	void SetPlaying(bool bPlaying) { m_bPlaying = bPlaying; }

	/** 跳到下一帧（仅当多帧图片时）
	*/
	bool IncrementCurrentFrame();

	/** 设置当前图片帧（仅当多帧图片时）
	*/
	void SetCurrentFrame(size_t nCurrentFrame);

	/** 获取当前图片帧索引（仅当多帧图片时）
	*/
	size_t GetCurrentFrameIndex() const;

	/** 获取当前图片帧的图片数据
	*/
	IBitmap* GetCurrentBitmap() const;

	/** 获取当前图片帧播放的时间间隔（单位: 毫秒，仅当多帧图片时）
	*/
	int GetCurrentInterval() const;

	/** 获取当前已循环播放的次数（仅当多帧图片时）
	*/
	int GetCycledCount() const;

	/** 清空当前已循环播放的次数（仅当多帧图片时）
	*/
	void ClearCycledCount();

	/** 判断是否应该继续播放（仅当多帧图片时）
	*/
	bool ContinuePlay() const;

private:

	//当前正在播放的图片帧（仅当多帧图片时）
	size_t m_nCurrentFrame;

	//是否正在播放（仅当多帧图片时）
	bool m_bPlaying;

	//已播放次数（仅当多帧图片时）
	int m_nCycledCount;

	//图片属性
	ImageAttribute m_imageAttribute;

	//图片信息
	std::shared_ptr<ImageInfo> m_imageCache;
};

/** 控件状态与图片的映射
*/
class UILIB_API StateImage
{
public:
	StateImage();

	/** 设置关联的控件接口
	*/
	void SetControl(Control* control) {	m_pControl = control; }

	/** 设置图片属性
	*@param [in] stateType 图片类型
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(ControlStateType stateType, const std::wstring& strImageString);

	/** 获取图片属性
	*@param [in] stateType 图片类型
	*/
	std::wstring GetImageString(ControlStateType stateType) const;

	/** 获取图片文件名
	*@param [in] stateType 图片类型
	*/
	std::wstring GetImagePath(ControlStateType stateType) const;

	/** 获取图片的源区域大小
	*@param [in] stateType 图片类型
	*/
	UiRect GetImageSourceRect(ControlStateType stateType) const;

	/** 获取图片的透明度
	*@param [in] stateType 图片类型
	*/
	int GetImageFade(ControlStateType stateType) const;

	/** 获取图片接口(可读，可写)
	*/
	Image& GetStateImage(ControlStateType stateType) { return m_stateImageMap[stateType]; }

public:
	/** 是否包含Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否包含状态图片
	*/
	bool HasImage() const;

	/** 绘制指定状态的图片
	*/
	bool PaintStateImage(IRender* pRender, ControlStateType stateType, const std::wstring& sImageModify = L"");

	/** 获取用于估算的图片接口
	*/
	Image* GetEstimateImage() ;

	/** 清空图片缓存，释放资源
	*/
	void ClearImageCache();

private:
	//关联的控件接口
	Control* m_pControl;

	//每个状态的图片接口
	std::map<ControlStateType, Image> m_stateImageMap;
};

/** 控件图片类型与状态图片的映射
*/
class UILIB_API StateImageMap
{
public:
	StateImageMap();

	/** 设置关联的控件接口
	*/
	void SetControl(Control* control);

	/** 设置图片属性
	*@param [in] stateImageType 图片类型，比如正常状态前景图片、背景图片；选择状态的前景图片、背景图片等
	*@param [in] stateType 图片状态，比如正常、焦点、按下、禁用状态等
	*@param [in] strImagePath 图片属性字符串
	*/
	void SetImageString(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath);

	/** 获取图片属性
	*@param [in] stateImageType 图片类型，比如正常状态前景图片、背景图片；选择状态的前景图片、背景图片等
	*@param [in] stateType 图片状态，比如正常、焦点、按下、禁用状态等
	*/
	std::wstring GetImageString(StateImageType stateImageType, ControlStateType stateType) const;

	/** 是否含有Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否含有指定类型的图片
	*/
	bool HasImageType(StateImageType stateImageType) const;

	/** 绘制指定图片类型和状态的图片
	*/
	bool PaintStateImage(IRender* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify = L"");
	
	/** 获取用于估算的图片接口
	*/
	Image* GetEstimateImage(StateImageType stateImageType);

	/** 清除所有图片类型的缓存，释放资源
	*/
	void ClearImageCache();

private:
	//关联的控件接口
	Control* m_pControl;

	//每个图片类型的状态图片(正常状态前景图片、背景图片；选择状态的前景图片、背景图片)
	std::map<StateImageType, StateImage> m_stateImageMap;
};

/** 控件状态与颜色值的映射
*/
class UILIB_API StateColorMap
{
public:
	StateColorMap();

	/** 设置管理的控件接口
	*/
	void SetControl(Control* control);

	/** 获取颜色值，如果不包含此颜色，则返回空
	*/
	std::wstring GetStateColor(ControlStateType stateType) const;

	/** 设置颜色值
	*/
	void SetStateColor(ControlStateType stateType, const std::wstring& color);

	/** 是否包含Hot状态的颜色
	*/
	bool HasHotColor() const;

	/** 是否含有指定颜色值
	*/
	bool HasStateColor(ControlStateType stateType) const;

	/** 是否含有颜色值
	*/
	bool HasStateColors() const ;

	/** 绘制指定状态的颜色
	*/
	void PaintStateColor(IRender* pRender, UiRect rcPaint, ControlStateType stateType) const;

private:
	//关联的控件接口
	Control* m_pControl;

	//状态与颜色值的映射表
	std::map<ControlStateType, std::wstring> m_stateColorMap;
};

} // namespace ui

#endif // UI_CORE_IMAGEDECODE_H_
