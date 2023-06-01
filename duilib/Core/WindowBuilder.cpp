#include "WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"
#include "duilib/Control/TreeView.h"
#include "duilib/Control/ScrollBar.h"
#include "duilib/Control/Combo.h"
#include "duilib/Control/Slider.h"
#include "duilib/Control/Progress.h"
#include "duilib/Control/CircleProgress.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/VirtualListBox.h"
#include "duilib/Control/VirtualTileBox.h"
#include "duilib/Control/DateTime.h"

#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/ChildBox.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Box/TileBox.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FontManager.h"

#include "duilib/third_party/xml/Markup.h"
#include <tchar.h>

namespace ui 
{

WindowBuilder::WindowBuilder()
{
	m_xml = std::make_unique<CMarkup>();
}

WindowBuilder::~WindowBuilder()
{
}

Box* WindowBuilder::Create(const std::wstring& xml, 
	                       CreateControlCallback pCallback,
						   Window* pWindow, 
	                       Box* pParent, 
	                       Box* pUserDefinedBox)
{
	ASSERT(!xml.empty() && L"xml 参数为空！");
	if (xml.empty()) {
		return nullptr;
	}
	//字符串以<开头认为是XML字符串，否则认为是XML文件
	//如果使用了 zip 压缩包，则从内存中读取
	if (xml.front() == L'<') {
		if (!m_xml->Load(xml.c_str())) {
			ASSERT(!L"Load xml 失败");
			return nullptr;
		}
	}
	else if (GlobalManager::IsUseZip()) {
		std::wstring sFile = GlobalManager::GetResourcePath();
		sFile += xml;

		std::vector<unsigned char> file_data;
		if (GlobalManager::GetZipData(sFile, file_data)) {
			bool ret = m_xml->LoadFromMem(file_data.data(), static_cast<DWORD>(file_data.size()));
			if (!ret) {
				ASSERT(!L"LoadFromMem 失败");
				return nullptr;
			}
		}
		else {
			if (!m_xml->LoadFromFile(xml.c_str())) {
				ASSERT(!L"LoadFromFile 失败");
				return nullptr;
			}
		}
	}
	else {
		if (!m_xml->LoadFromFile(xml.c_str())) {
			ASSERT(!L"LoadFromFile 失败");
			return nullptr;
		}
	}
	return Create(pCallback, pWindow, pParent, pUserDefinedBox);
}

Box* WindowBuilder::Create(CreateControlCallback pCallback, Window* pWindow, Box* pParent, Box* pUserDefinedBox)
{
	m_createControlCallback = pCallback;
	CMarkupNode root = m_xml->GetRoot();
	ASSERT(root.IsValid());
	if (!root.IsValid()) {
		return nullptr;
	}

	if( pWindow != nullptr) {
		std::wstring strClass;
		std::wstring strName;
		std::wstring strValue;
		strClass = root.GetName();

		if( strClass == L"Global") {
			int nAttributes = root.GetAttributeCount();
			for( int i = 0; i < nAttributes; i++ ) {
				strName = root.GetAttributeName(i);
				strValue = root.GetAttributeValue(i);
				if( strName == _T("disabledfontcolor") ) {
					GlobalManager::SetDefaultDisabledTextColor(strValue);
				} 
				else if( strName == _T("defaultfontcolor") ) {	
					GlobalManager::SetDefaultTextColor(strValue);
				}
				else if( strName == _T("linkfontcolor") ) {
					UiColor clrColor = GlobalManager::GetTextColor(strValue);
					GlobalManager::SetDefaultLinkFontColor(clrColor);
				} 
				else if( strName == _T("linkhoverfontcolor") ) {
					UiColor clrColor = GlobalManager::GetTextColor(strValue);
					GlobalManager::SetDefaultLinkHoverFontColor(clrColor);
				} 
				else if( strName == _T("selectedcolor") ) {
					UiColor clrColor = GlobalManager::GetTextColor(strValue);
					GlobalManager::SetDefaultSelectedBkColor(clrColor);
				}
			}
		}
		else if( strClass == _T("Window") ) {
			if( pWindow->GetHWND() ) {
				int nAttributes = root.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) {
					strName = root.GetAttributeName(i);
					strValue = root.GetAttributeValue(i);
					if( strName == _T("size") ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(strValue.c_str(), &pstr, 10);	ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);	ASSERT(pstr); 
						pWindow->SetInitSize(cx, cy);
					} 
					else if( strName == _T("heightpercent") ) {
						double lfHeightPercent = _ttof(strValue.c_str());
	
						MONITORINFO oMonitor = {}; 
						oMonitor.cbSize = sizeof(oMonitor);
						::GetMonitorInfo(::MonitorFromWindow(pWindow->GetHWND(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
						int nWindowHeight = int((oMonitor.rcWork.bottom - oMonitor.rcWork.top) * lfHeightPercent);
						int nMinHeight = pWindow->GetMinInfo().cy;
						int nMaxHeight = pWindow->GetMaxInfo().cy;
						if (nMinHeight != 0 && nWindowHeight < nMinHeight) {
							nWindowHeight = nMinHeight;
						}
						if (nMaxHeight != 0 && nWindowHeight > nMaxHeight) {
							nWindowHeight = nMaxHeight;
						}

						UiSize xy = pWindow->GetInitSize();
						pWindow->SetInitSize(xy.cx, nWindowHeight, false, false);
					}
					else if( strName == _T("sizebox") ) {
						UiRect rcSizeBox;
						LPTSTR pstr = NULL;
						rcSizeBox.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
						rcSizeBox.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcSizeBox.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcSizeBox.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
						pWindow->SetSizeBox(rcSizeBox);
					}
					else if( strName == _T("caption") ) {
						UiRect rcCaption;
						LPTSTR pstr = NULL;
						rcCaption.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
						rcCaption.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcCaption.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcCaption.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
						pWindow->SetCaptionRect(rcCaption);
					}
					else if( strName == _T("text") ) {
						pWindow->SetText(strValue);
					}
					else if (strName == _T("textid")) {
						pWindow->SetTextId(strValue);
					}
					else if( strName == _T("roundcorner") ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pWindow->SetRoundCorner(cx, cy);
					} 
					else if( strName == _T("mininfo") ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pWindow->SetMinInfo(cx, cy);
					}
					else if( strName == _T("maxinfo") ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pWindow->SetMaxInfo(cx, cy);
					}					
					else if (strName == _T("alphafixcorner") || strName == _T("custom_shadow")) {
						UiRect rc;
						LPTSTR pstr = NULL;
						rc.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
						rc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
						rc.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
						rc.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
						pWindow->SetAlphaFixCorner(rc);
					}
					else if (strName == _T("renderalpha")) {
						pWindow->SetRenderTransparent(strValue == _T("true"));
					}
					else if ((strName == _T("shadow_attached")) || (strName == _T("shadowattached"))) {
						//设置是否支持窗口阴影（阴影实现有两种：层窗口和普通窗口）
						pWindow->SetShadowAttached(strValue == _T("true"));
					}
					else if ((strName == _T("shado_wimage")) || (strName == _T("shadowimage"))) {
						//设置阴影图片
						pWindow->SetShadowImage(strValue);
					}
					else if ((strName == _T("shadow_corner")) || (strName == _T("shadowcorner"))) {
						//设置窗口阴影的九宫格属性
						UiRect rc;
						LPTSTR pstr = NULL;
						rc.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
						rc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
						rc.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
						rc.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
						pWindow->SetShadowCorner(rc);
					}
					else if (strName == _T("layered_window")) {
						//设置是否设置层窗口属性（层窗口还是普通窗口）
						pWindow->SetLayeredWindow(strValue == _T("true"));
					}
					else if (strName == _T("alpha")) {
						//设置窗口的透明度（0 - 255），仅当使用层窗口时有效
						LPTSTR pstr = NULL;
						int nAlpha = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
						ASSERT(nAlpha >= 0 && nAlpha <= 255);
						if ((nAlpha >= 0) && (nAlpha <= 255)) {
							pWindow->SetWindowAlpha(nAlpha);
						}
					}
				}
			}
		}

		if( strClass == _T("Global") ) {
			for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
				strClass = node.GetName();
				if( strClass == _T("Image") ) {
					ASSERT(FALSE);	//废弃
				}
				else if (strClass == _T("FontResource")) {
					int nAttributes = node.GetAttributeCount();
					std::wstring strFontFile;
					std::wstring strFontName;
					for (int i = 0; i < nAttributes; i++) {
						strName = node.GetAttributeName(i);
						strValue = node.GetAttributeValue(i);
						if (strName == _T("file")) {
							strFontFile = strValue;
						}
						else if (strName == _T("name")) {
							strFontName = strValue;
						}
					}
					if (!strFontFile.empty()) {
						FontManager::GetInstance()->AddFontResource(strFontFile, strFontName);
					}
				}
				else if( strClass == _T("Font") ) {
					int nAttributes = node.GetAttributeCount();
					std::wstring strFontId;
					std::wstring strFontName;
					int size = 12;
					int weight = 0;
					bool bold = false;
					bool underline = false;
					bool strikeout = false;
					bool italic = false;
					bool isDefault = false;
					for( int i = 0; i < nAttributes; i++ ) {
						strName = node.GetAttributeName(i);
						strValue = node.GetAttributeValue(i);
						if (strName == _T("id"))
						{
							strFontId = strValue;
						}
						else if( strName == _T("name") ) {
							strFontName = strValue;
						}
						else if( strName == _T("size") ) {
							size = _tcstol(strValue.c_str(), NULL, 10);
						}
						else if( strName == _T("bold") ) {
							bold = (strValue == _T("true"));
						}
						else if( strName == _T("underline") ) {
							underline = (strValue == _T("true"));
						}
						else if (strName == _T("strikeout")) {
							strikeout = (strValue == _T("true"));
						}
						else if( strName == _T("italic") ) {
							italic = (strValue == _T("true"));
						}
						else if( strName == _T("default") ) {
							isDefault = (strValue == _T("true"));
						}
						else if ( strName == _T("weight") ) {
							weight = _tcstol(strValue.c_str(), NULL, 10);
						}
					}
					if( !strFontName.empty() ) {
						GlobalManager::AddFont(strFontId, strFontName, size, bold, underline, strikeout, italic, isDefault, weight);
					}
				}
				else if( strClass == _T("Class") ) {
					int nAttributes = node.GetAttributeCount();
					std::wstring strClassName;
					std::wstring strAttribute;
					for( int i = 0; i < nAttributes; i++ ) {
						strName = node.GetAttributeName(i);
						strValue = node.GetAttributeValue(i);
						if( strName == _T("name") ) {
							strClassName = strValue;
						}
						else if( strName == _T("value") ) {
							strAttribute.append(strValue);
						}
						else if (strName == _T("_value")) {
							strAttribute.append(StringHelper::Printf(L" value=\"%s\"",strValue.c_str()));
						}
						else {
							strAttribute.append(StringHelper::Printf(L" %s=\"%s\"",
								strName.c_str(), strValue.c_str()));
						}
					}
					if( !strClassName.empty() ) {
						StringHelper::TrimLeft(strAttribute);
						GlobalManager::AddClass(strClassName, strAttribute);
					}
				}
				else if( strClass == _T("TextColor") ) {
					int nAttributes = node.GetAttributeCount();
					std::wstring strColorName;
					std::wstring strColor;
					for( int i = 0; i < nAttributes; i++ ) {
						strName = node.GetAttributeName(i);
						strValue = node.GetAttributeValue(i);
						if( strName == _T("name") ) {
							strColorName = strValue;
						}
						else if( strName == _T("value") ) {
							strColor = strValue;
						}
					}
					if( !strColorName.empty()) {
						GlobalManager::AddTextColor(strColorName, strColor);
					}
				}
			}
		}
		else if ( strClass == _T("Window") )
		{
			for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
				strClass = node.GetName();
				if( strClass == _T("Class") ) {
					int nAttributes = node.GetAttributeCount();
					std::wstring strClassName;
					std::wstring strAttribute;
					for( int i = 0; i < nAttributes; i++ ) {
						strName = node.GetAttributeName(i);
						strValue = node.GetAttributeValue(i);
						if( strName == _T("name") ) {
							strClassName = strValue;
						}
						else if( strName == _T("value") ) {
							strAttribute.append(strValue);
						}
						else if (strName == _T("_value")) {
							strAttribute.append(StringHelper::Printf(L" value=\"%s\"", strValue.c_str()));
						}
						else {
							strAttribute.append(StringHelper::Printf(L" %s=\"%s\"",
								strName.c_str(), strValue.c_str()));
						}
					}
					if( !strClassName.empty() ) {
						ASSERT( GlobalManager::GetClassAttributes(strClassName).empty() );	//窗口中的Class不能与全局的重名
						StringHelper::TrimLeft(strAttribute);
						pWindow->AddClass(strClassName, strAttribute);
					}
				}
				else if (strClass == _T("TextColor")) {
					int nAttributes = node.GetAttributeCount();
					std::wstring strColorName;
					std::wstring strColor;
					for (int i = 0; i < nAttributes; i++) {
						strName = node.GetAttributeName(i);
						strValue = node.GetAttributeValue(i);
						if (strName == _T("name")) {
							strColorName = strValue;
						}
						else if (strName == _T("value")) {
							strColor = strValue;
						}
					}
					if (!strColorName.empty()) {
						pWindow->AddTextColor(strColorName, strColor);
					}
				}
			}
		}
	}

	for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
		std::wstring strClass = node.GetName();
		if (strClass == _T("Image") || strClass == _T("FontResource") || strClass == _T("Font")
			|| strClass == _T("Class") || strClass == _T("TextColor") ) {

		}
		else {
			if (!pUserDefinedBox) {
				return (Box*)_Parse(&root, pParent, pWindow);
			}
			else {
				_Parse(&node, pUserDefinedBox, pWindow);

				int nAttributes = node.GetAttributeCount();
				for (int i = 0; i < nAttributes; i++) {
					ASSERT(i == 0 || _tcscmp(node.GetAttributeName(i), _T("class")) != 0);	//class必须是第一个属性
					pUserDefinedBox->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
				}
				return pUserDefinedBox;
			}
		}
	}

	return nullptr;
}

Control* WindowBuilder::_Parse(CMarkupNode* pRoot, Control* pParent, Window* pWindow)
{
	ASSERT(pRoot != nullptr);
	if (pRoot == nullptr) {
		return nullptr;
	}
    Control* pReturn = NULL;
    for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
        std::wstring strClass = node.GetName();
		if( (strClass == L"Image") || 
			(strClass == L"Font")  ||
			(strClass == L"Class") || 
			(strClass == L"TextColor") ) {
				continue;
		}

        Control* pControl = NULL;
        if (strClass == L"Include") {
			if (!node.HasAttributes()) {
				continue;
			}
            int nCount = 1;
            LPTSTR pstr = NULL;
            TCHAR szValue[500] = { 0 };
            SIZE_T cchLen = sizeof(szValue)/sizeof(szValue[0]) - 1;
			if (node.GetAttributeValue(L"count", szValue, cchLen)) {
				nCount = _tcstol(szValue, &pstr, 10);
			}
            cchLen = sizeof(szValue) / sizeof(szValue[0]) - 1;
			if (!node.GetAttributeValue(L"source", szValue, cchLen)) {
				continue;
			}
            for ( int i = 0; i < nCount; i++ ) {
                WindowBuilder builder;
                pControl = builder.Create(szValue, m_createControlCallback, pWindow, (Box*)pParent);
            }
            continue;
        }
        else {
			pControl = CreateControlByClass(strClass);
			if (pControl == nullptr) {
				if ((strClass == L"Event") || 
					(strClass == L"BubbledEvent")) {
					bool bBubbled = (strClass == L"BubbledEvent");
					AttachXmlEvent(bBubbled, node, pParent);
					continue;
				}
			}

            // User-supplied control factory
            if( pControl == NULL ) {
				pControl = GlobalManager::CreateControl(strClass);
            }

            if( pControl == NULL && m_createControlCallback ) {
                pControl = m_createControlCallback(strClass);
            }
        }

		if( pControl == NULL ) {
			ASSERT(FALSE);
			continue;
		}

		// TreeView相关必须先添加后解析
		if (strClass == DUI_CTR_TREENODE) {
			TreeNode* pNode = dynamic_cast<TreeNode*>(pControl);
			ASSERT(pNode != nullptr);
			TreeView* pTreeView = dynamic_cast<TreeView*>(pParent);
			ASSERT(pTreeView != nullptr);
			if (pTreeView) {
				pTreeView->GetRootNode()->AddChildNode(pNode);
			}
			else {
				TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pParent);
				ASSERT(pTreeNode != nullptr);
				if (pTreeNode) {
					pTreeNode->AddChildNode(pNode);
				}
			}
		}

		pControl->SetWindow(pWindow);
		// Add children
		if (node.HasChildren()) {
			_Parse(&node, (Box*)pControl, pWindow);
		}

		// Process attributes
		if( node.HasAttributes() ) {
			// Set ordinary attributes
			int nAttributes = node.GetAttributeCount();
			for( int i = 0; i < nAttributes; i++ ) {
				ASSERT(i == 0 || _tcscmp(node.GetAttributeName(i), L"class") != 0);	//class必须是第一个属性
				pControl->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
			}
		}

		// Attach to parent
        // 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
		if (pParent != NULL && strClass != DUI_CTR_TREENODE) {
			Box* pContainer = dynamic_cast<Box*>(pParent);
			ASSERT(pContainer);
			if( pContainer == NULL ) return NULL;
			if( !pContainer->AddItem(pControl) ) {
				ASSERT(FALSE);
				delete pControl;
				continue;
			}
		}
        
        // Return first item
		if (pReturn == nullptr) {
			pReturn = pControl;
		}
    }
    return pReturn;
}

Control* WindowBuilder::CreateControlByClass(const std::wstring& strControlClass)
{
	Control* pControl = nullptr;
	size_t cchLen = strControlClass.size();
	switch( cchLen ) {
	case 3:
		if( strControlClass == DUI_CTR_BOX )					pControl = new Box;
		break;
	case 4:
		if( strControlClass == DUI_CTR_HBOX )					pControl = new HBox;
		else if( strControlClass == DUI_CTR_VBOX )				pControl = new VBox;
		break;
	case 5:
		if( strControlClass == DUI_CTR_COMBO )                  pControl = new Combo;
		else if( strControlClass == DUI_CTR_LABEL )             pControl = new Label;
		break;
	case 6:
		if( strControlClass == DUI_CTR_BUTTON )                 pControl = new Button;
		else if( strControlClass == DUI_CTR_OPTION )            pControl = new Option;
		else if( strControlClass == DUI_CTR_SLIDER )            pControl = new Slider;
		else if( strControlClass == DUI_CTR_TABBOX )			pControl = new TabBox;
		break;
	case 7:
		if( strControlClass == DUI_CTR_CONTROL )                pControl = new Control;
		else if( strControlClass == DUI_CTR_TILEBOX )		  	pControl = new TileBox;
		else if (strControlClass == DUI_CTR_LISTBOX)			pControl = new ListBox(new Layout);
		break;
	case 8:
		if( strControlClass == DUI_CTR_PROGRESS )               pControl = new Progress;
		else if( strControlClass == DUI_CTR_RICHEDIT )          pControl = new RichEdit;
		else if( strControlClass == DUI_CTR_CHECKBOX )			pControl = new CheckBox;
		else if( strControlClass == DUI_CTR_TREEVIEW )			pControl = new TreeView;
		else if( strControlClass == DUI_CTR_TREENODE )			pControl = new TreeNode;
		else if( strControlClass == DUI_CTR_HLISTBOX )			pControl = new ListBox(new HLayout);
		else if( strControlClass == DUI_CTR_VLISTBOX )          pControl = new ListBox(new VLayout);
		else if( strControlClass == DUI_CTR_CHILDBOX )			pControl = new ChildBox;
		else if( strControlClass == DUI_CTR_LABELBOX )          pControl = new LabelBox;
		else if( strControlClass == DUI_CTR_DATETIME)			pControl = new DateTime;
		break;
	case 9:
		if( strControlClass == DUI_CTR_SCROLLBAR )				pControl = new ScrollBar; 
		else if( strControlClass == DUI_CTR_BUTTONBOX )         pControl = new ButtonBox;
		else if( strControlClass == DUI_CTR_OPTIONBOX )         pControl = new OptionBox;
		break;
	case 11:
		if( strControlClass == DUI_CTR_TILELISTBOX )			pControl = new ListBox(new TileLayout);
		else if( strControlClass == DUI_CTR_CHECKBOXBOX )		pControl = new CheckBoxBox;
		break;
	case 14:
    if (strControlClass == DUI_CTR_VIRTUALLISTBOX)			pControl = new VirtualListBox;
    else if (strControlClass == DUI_CTR_CIRCLEPROGRESS)     pControl = new CircleProgress;
    else if (strControlClass == DUI_CTR_VIRTUALTILEBOX)     pControl = new VirtualTileBox;
		break;
	case 15:
		break;
	case 16:
		break;
	case 20:
		if( strControlClass == DUI_CTR_LISTBOX_ELEMENT)   pControl = new ListBoxElement;
		break;
	default:
		break;
	}

	return pControl;
}

void WindowBuilder::AttachXmlEvent(bool bBubbled, CMarkupNode& node, Control* pParent)
{
	ASSERT(pParent != nullptr);
	if (pParent == nullptr) {
		return;
	}
	auto nAttributes = node.GetAttributeCount();
	std::wstring strType;
	std::wstring strReceiver;
	std::wstring strApplyAttribute;
	std::wstring strName;
	std::wstring strValue;
	for( int i = 0; i < nAttributes; i++ ) {
		strName = node.GetAttributeName(i);
		strValue = node.GetAttributeValue(i);
		ASSERT(i != 0 || strName == _T("type"));
		ASSERT(i != 1 || strName == _T("receiver"));
		ASSERT(i != 2 || strName == _T("applyattribute"));
		if( strName == _T("type") ) {
			strType = strValue;
		}
		else if( strName == _T("receiver") ) {
			strReceiver = strValue;
		}
		else if( strName == _T("applyattribute") ) {
			strApplyAttribute = strValue;
		}
	}

	auto typeList = StringHelper::Split(strType, L" ");
	auto receiverList = StringHelper::Split(strReceiver, L" ");
	for (auto itType = typeList.begin(); itType != typeList.end(); itType++) {
		for (auto itReceiver = receiverList.begin(); itReceiver != receiverList.end(); itReceiver++) {
			EventType eventType = StringToEnum(*itType);
			auto callback = nbase::Bind(&Control::OnApplyAttributeList, pParent, *itReceiver, strApplyAttribute, std::placeholders::_1);
			if (!bBubbled) {
				pParent->AttachXmlEvent(eventType, callback);
			}
			else {
				pParent->AttachXmlBubbledEvent(eventType, callback);
			}
		}
	}
}

} // namespace ui
