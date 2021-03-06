/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 *   Mozilla Foundation
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Vladimir Vukicevic <vladimir@pobox.com>
 *   Alex Pakhotin <alexp@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "mozilla/dom/ContentChild.h"
#include "nsStyleConsts.h"
#include "nsXULAppAPI.h"
#include "nsLookAndFeel.h"

using namespace mozilla;
using mozilla::dom::ContentChild;

bool nsLookAndFeel::mInitializedSystemColors = false;
AndroidSystemColors nsLookAndFeel::mSystemColors;

bool nsLookAndFeel::mInitializedShowPassword = false;
bool nsLookAndFeel::mShowPassword = true;

nsLookAndFeel::nsLookAndFeel()
    : nsXPLookAndFeel()
{
}

nsLookAndFeel::~nsLookAndFeel()
{
}

#define BG_PRELIGHT_COLOR      NS_RGB(0xee,0xee,0xee)
#define FG_PRELIGHT_COLOR      NS_RGB(0x77,0x77,0x77)
#define BLACK_COLOR            NS_RGB(0x00,0x00,0x00)
#define DARK_GRAY_COLOR        NS_RGB(0x40,0x40,0x40)
#define GRAY_COLOR             NS_RGB(0x80,0x80,0x80)
#define LIGHT_GRAY_COLOR       NS_RGB(0xa0,0xa0,0xa0)
#define RED_COLOR              NS_RGB(0xff,0x00,0x00)

nsresult
nsLookAndFeel::GetSystemColors()
{
    if (mInitializedSystemColors)
        return NS_OK;

    if (!AndroidBridge::Bridge())
        return NS_ERROR_FAILURE;

    AndroidBridge::Bridge()->GetSystemColors(&mSystemColors);

    mInitializedSystemColors = true;

    return NS_OK;
}

nsresult
nsLookAndFeel::CallRemoteGetSystemColors()
{
    // An array has to be used to get data from remote process
    InfallibleTArray<PRUint32> colors;
    PRUint32 colorsCount = sizeof(AndroidSystemColors) / sizeof(nscolor);

    if (!ContentChild::GetSingleton()->SendGetSystemColors(colorsCount, &colors))
        return NS_ERROR_FAILURE;

    NS_ASSERTION(colors.Length() == colorsCount, "System colors array is incomplete");
    if (colors.Length() == 0)
        return NS_ERROR_FAILURE;

    if (colors.Length() < colorsCount)
        colorsCount = colors.Length();

    // Array elements correspond to the members of mSystemColors structure,
    // so just copy the memory block
    memcpy(&mSystemColors, colors.Elements(), sizeof(nscolor) * colorsCount);

    mInitializedSystemColors = true;

    return NS_OK;
}

nsresult
nsLookAndFeel::NativeGetColor(ColorID aID, nscolor &aColor)
{
    nsresult rv = NS_OK;

    if (!mInitializedSystemColors) {
        if (XRE_GetProcessType() == GeckoProcessType_Default)
            rv = GetSystemColors();
        else
            rv = CallRemoteGetSystemColors();
        NS_ENSURE_SUCCESS(rv, rv);
    }

    // XXX we'll want to use context.obtainStyledAttributes on the java side to
    // get all of these; see TextView.java for a good exmaple.

    switch (aID) {
        // These colors don't seem to be used for anything anymore in Mozilla
        // (except here at least TextSelectBackground and TextSelectForeground)
        // The CSS2 colors below are used.
    case eColorID_WindowBackground:
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_WindowForeground:
        aColor = mSystemColors.textColorPrimary;
        break;
    case eColorID_WidgetBackground:
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_WidgetForeground:
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID_WidgetSelectBackground:
        aColor = mSystemColors.textColorHighlight;
        break;
    case eColorID_WidgetSelectForeground:
        aColor = mSystemColors.textColorPrimaryInverse;
        break;
    case eColorID_Widget3DHighlight:
        aColor = LIGHT_GRAY_COLOR;
        break;
    case eColorID_Widget3DShadow:
        aColor = DARK_GRAY_COLOR;
        break;
    case eColorID_TextBackground:
        // not used?
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_TextForeground:
        // not used?
        aColor = mSystemColors.textColorPrimary;
        break;
    case eColorID_TextSelectBackground:
    case eColorID_IMESelectedRawTextBackground:
    case eColorID_IMESelectedConvertedTextBackground:
        // still used
        aColor = mSystemColors.textColorHighlight;
        break;
    case eColorID_TextSelectForeground:
    case eColorID_IMESelectedRawTextForeground:
    case eColorID_IMESelectedConvertedTextForeground:
        // still used
        aColor = mSystemColors.textColorPrimaryInverse;
        break;
    case eColorID_IMERawInputBackground:
    case eColorID_IMEConvertedTextBackground:
        aColor = NS_TRANSPARENT;
        break;
    case eColorID_IMERawInputForeground:
    case eColorID_IMEConvertedTextForeground:
        aColor = NS_SAME_AS_FOREGROUND_COLOR;
        break;
    case eColorID_IMERawInputUnderline:
    case eColorID_IMEConvertedTextUnderline:
        aColor = NS_SAME_AS_FOREGROUND_COLOR;
        break;
    case eColorID_IMESelectedRawTextUnderline:
    case eColorID_IMESelectedConvertedTextUnderline:
        aColor = NS_TRANSPARENT;
        break;
    case eColorID_SpellCheckerUnderline:
      aColor = RED_COLOR;
      break;

        // css2  http://www.w3.org/TR/REC-CSS2/ui.html#system-colors
    case eColorID_activeborder:
        // active window border
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_activecaption:
        // active window caption background
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_appworkspace:
        // MDI background color
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_background:
        // desktop background
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_captiontext:
        // text in active window caption, size box, and scrollbar arrow box (!)
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID_graytext:
        // disabled text in windows, menus, etc.
        aColor = mSystemColors.textColorTertiary;
        break;
    case eColorID_highlight:
        // background of selected item
        aColor = mSystemColors.textColorHighlight;
        break;
    case eColorID_highlighttext:
        // text of selected item
        aColor = mSystemColors.textColorPrimaryInverse;
        break;
    case eColorID_inactiveborder:
        // inactive window border
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_inactivecaption:
        // inactive window caption
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_inactivecaptiontext:
        // text in inactive window caption
        aColor = mSystemColors.textColorTertiary;
        break;
    case eColorID_infobackground:
        // tooltip background color
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_infotext:
        // tooltip text color
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID_menu:
        // menu background
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID_menutext:
        // menu text
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID_scrollbar:
        // scrollbar gray area
        aColor = mSystemColors.colorBackground;
        break;

    case eColorID_threedface:
    case eColorID_buttonface:
        // 3-D face color
        aColor = mSystemColors.colorBackground;
        break;

    case eColorID_buttontext:
        // text on push buttons
        aColor = mSystemColors.colorForeground;
        break;

    case eColorID_buttonhighlight:
        // 3-D highlighted edge color
    case eColorID_threedhighlight:
        // 3-D highlighted outer edge color
        aColor = LIGHT_GRAY_COLOR;
        break;

    case eColorID_threedlightshadow:
        // 3-D highlighted inner edge color
        aColor = mSystemColors.colorBackground;
        break;

    case eColorID_buttonshadow:
        // 3-D shadow edge color
    case eColorID_threedshadow:
        // 3-D shadow inner edge color
        aColor = GRAY_COLOR;
        break;

    case eColorID_threeddarkshadow:
        // 3-D shadow outer edge color
        aColor = BLACK_COLOR;
        break;

    case eColorID_window:
    case eColorID_windowframe:
        aColor = mSystemColors.colorBackground;
        break;

    case eColorID_windowtext:
        aColor = mSystemColors.textColorPrimary;
        break;

    case eColorID__moz_eventreerow:
    case eColorID__moz_field:
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID__moz_fieldtext:
        aColor = mSystemColors.textColorPrimary;
        break;
    case eColorID__moz_dialog:
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID__moz_dialogtext:
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID__moz_dragtargetzone:
        aColor = mSystemColors.textColorHighlight;
        break;
    case eColorID__moz_buttondefault:
        // default button border color
        aColor = BLACK_COLOR;
        break;
    case eColorID__moz_buttonhoverface:
        aColor = BG_PRELIGHT_COLOR;
        break;
    case eColorID__moz_buttonhovertext:
        aColor = FG_PRELIGHT_COLOR;
        break;
    case eColorID__moz_cellhighlight:
    case eColorID__moz_html_cellhighlight:
        aColor = mSystemColors.textColorHighlight;
        break;
    case eColorID__moz_cellhighlighttext:
    case eColorID__moz_html_cellhighlighttext:
        aColor = mSystemColors.textColorPrimaryInverse;
        break;
    case eColorID__moz_menuhover:
        aColor = BG_PRELIGHT_COLOR;
        break;
    case eColorID__moz_menuhovertext:
        aColor = FG_PRELIGHT_COLOR;
        break;
    case eColorID__moz_oddtreerow:
        aColor = NS_TRANSPARENT;
        break;
    case eColorID__moz_nativehyperlinktext:
        aColor = NS_SAME_AS_FOREGROUND_COLOR;
        break;
    case eColorID__moz_comboboxtext:
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID__moz_combobox:
        aColor = mSystemColors.colorBackground;
        break;
    case eColorID__moz_menubartext:
        aColor = mSystemColors.colorForeground;
        break;
    case eColorID__moz_menubarhovertext:
        aColor = FG_PRELIGHT_COLOR;
        break;
    default:
        /* default color is BLACK */
        aColor = 0;
        rv = NS_ERROR_FAILURE;
        break;
    }

    return rv;
}


nsresult
nsLookAndFeel::GetIntImpl(IntID aID, PRInt32 &aResult)
{
    nsresult rv = nsXPLookAndFeel::GetIntImpl(aID, aResult);
    if (NS_SUCCEEDED(rv))
        return rv;

    rv = NS_OK;

    switch (aID) {
        case eIntID_CaretBlinkTime:
            aResult = 500;
            break;

        case eIntID_CaretWidth:
            aResult = 1;
            break;

        case eIntID_ShowCaretDuringSelection:
            aResult = 0;
            break;

        case eIntID_SelectTextfieldsOnKeyFocus:
            // Select textfield content when focused by kbd
            // used by nsEventStateManager::sTextfieldSelectModel
            aResult = 1;
            break;

        case eIntID_SubmenuDelay:
            aResult = 200;
            break;

        case eIntID_MenusCanOverlapOSBar:
            // we want XUL popups to be able to overlap the task bar.
            aResult = 1;
            break;

        case eIntID_ScrollArrowStyle:
            aResult = eScrollArrowStyle_Single;
            break;

        case eIntID_ScrollSliderStyle:
            aResult = eScrollThumbStyle_Proportional;
            break;

        case eIntID_WindowsDefaultTheme:
        case eIntID_TouchEnabled:
        case eIntID_MaemoClassic:
        case eIntID_WindowsThemeIdentifier:
            aResult = 0;
            rv = NS_ERROR_NOT_IMPLEMENTED;
            break;

        case eIntID_SpellCheckerUnderlineStyle:
            aResult = NS_STYLE_TEXT_DECORATION_STYLE_WAVY;
            break;

        default:
            aResult = 0;
            rv = NS_ERROR_FAILURE;
    }

    return rv;
}

nsresult
nsLookAndFeel::GetFloatImpl(FloatID aID, float &aResult)
{
    nsresult rv = nsXPLookAndFeel::GetFloatImpl(aID, aResult);
    if (NS_SUCCEEDED(rv))
        return rv;
    rv = NS_OK;

    switch (aID) {
        case eFloatID_IMEUnderlineRelativeSize:
            aResult = 1.0f;
            break;

        case eFloatID_SpellCheckerUnderlineRelativeSize:
            aResult = 1.0f;
            break;

        default:
            aResult = -1.0;
            rv = NS_ERROR_FAILURE;
            break;
    }
    return rv;
}

/*virtual*/
bool
nsLookAndFeel::GetEchoPasswordImpl()
{
    if (!mInitializedShowPassword) {
        if (XRE_GetProcessType() == GeckoProcessType_Default) {
            if (AndroidBridge::Bridge())
                mShowPassword = AndroidBridge::Bridge()->GetShowPasswordSetting();
            else
                NS_ASSERTION(AndroidBridge::Bridge() != nsnull, "AndroidBridge is not available!");
        } else {
            ContentChild::GetSingleton()->SendGetShowPasswordSetting(&mShowPassword);
        }
        mInitializedShowPassword = true;
    }
    return mShowPassword;
}
