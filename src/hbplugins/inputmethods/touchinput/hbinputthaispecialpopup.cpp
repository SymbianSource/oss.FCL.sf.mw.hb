/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

#include <QGraphicsGridLayout>
#include <QGraphicsWidget>
#include <QString>
#include <QSignalMapper>

#include <HbMainWindow>
#include <hbinputtouchkeypadbutton.h>
#include <hbgridview.h>
#include <hbwidget.h>
#include <hbdialog_p.h>
#include <HbPopup>

#include "hbinputthaispecialpopup.h"

const QString HbAbcButtonObjName = "Popup abc";

const int HbPopupAsteriskColumns = 5;
const int HbPopupAsteriskRows = 4;
const int HbPopupShiftColumns = 4;
const int HbPopupShiftRows = 3;


const int HBPopupEmptyButtonSeven = 7;
const int HBPopupEmptyButtonEight = 8;
const int HBPopupEmptyButtonNine = 9;

const QString HbPopupPortraitButtonTextLayout = "_hb_sctp_button_text_layout";
const QString HbPopupPortraitButtonIconLayout = "_hb_sctp_button_icon_layout";

/*!
@proto
@hbtouchinput
\class HbInputThaiSpecialPopup
\brief Implementation of Thai Special Popup.

Implementation of Thai Special Popup.

*/

/// @cond

class HbInputThaiSpecialPopupPrivate: public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbInputThaiSpecialPopup)

public:
	//Character classes
	enum ThaiGlyphTypes {
		HbThaiCharNon = 0,	//Not a Thai letter
		HbThaiCharCons,		//Thai consonant
		HbThaiCharLV,		//Leading vowel
		HbThaiCharFV1,		//Following vowel, type 1
		HbThaiCharFV2,		//Following vowel, type 2
		HbThaiCharFV3,		//Following vowel, type 3
		HbThaiCharBV1,		//Below vowel, type 1
		HbThaiCharBV2,		//Below vowel, type 2
		HbThaiCharBD,		//Below diacritic
		HbThaiCharTone,		//Tone mark
		HbThaiCharAD1,		//Above diacritic, type 1
		HbThaiCharAD2,		//Above diacritic, type 2
		HbThaiCharAD3,		//Above diacritic, type 3
		HbThaiCharAV1,		//Above vowel, type 1
		HbThaiCharAV2,		//Above vowel, type 2
		HbThaiCharAV3,		//Above vowel, type 3
		HbThaiCharNonThai	//Not a Thai letter
	};
	QGraphicsGridLayout* mButtonLayout; 
	QGraphicsWidget* mButtonWidget;
	QList<HbTouchKeypadButton *> mPopupButtons;
    QSignalMapper *mActionMapper;
	QSignalMapper *mClickMapper;
	QString mSpecialCharacterSet;
	const HbKeymap *mKeymap;
	QSizeF mSize;
	int mButtonId;
	int mPopupGridColumns;
	int mPopupGridRows;
	int mNumPopupButtons;
	int mAbcButtonId;
	uint mPrevChar;


public:
	HbInputThaiSpecialPopupPrivate();
	~HbInputThaiSpecialPopupPrivate();
	void setNumberOfKeys();
	void createPopupButtons(int screenWidth, int screenHeight);
	void setLayoutDimensions(QSizeF dimensions);
	void getSpecialCharacters();
	void setPopupButtons(const QString &aCharSet);
	void applyEditorConstraints();
	int thaiGlyphType(uint prevChar);	
	void initializeAbcButton();	
	void handleStandardButtonClick(int buttonId);
	void _q_mappedKeyClick(int buttonid);	
};

HbInputThaiSpecialPopupPrivate::HbInputThaiSpecialPopupPrivate()
{
    // we should make sure that it comes above vkb
    setPriority(HbPopupPrivate::VirtualKeyboard + 1);
	mClickMapper = 0;
	mSize = QSizeF();
}

HbInputThaiSpecialPopupPrivate::~HbInputThaiSpecialPopupPrivate()
{
}

/*!
Sets number of keys to layout
*/
void HbInputThaiSpecialPopupPrivate::setNumberOfKeys()
{
	//Manipulate number of keys on the layout depending on the Key_Asterisk and Qt::Key_Shift
	if(Qt::Key_Asterisk == mButtonId) {
		mPopupGridColumns = HbPopupAsteriskColumns;
		mPopupGridRows = HbPopupAsteriskRows;
	}else if (Qt::Key_Shift == mButtonId) {
		mPopupGridColumns = HbPopupShiftColumns;
		mPopupGridRows = HbPopupShiftRows;
	}
	mNumPopupButtons = mPopupGridColumns * mPopupGridRows;
	mAbcButtonId = mPopupGridColumns*mPopupGridRows-1;
}


/*!
Create Popup Buttons
*/
void HbInputThaiSpecialPopupPrivate::createPopupButtons(int screenWidth, int screenHeight)
{
	Q_Q(HbInputThaiSpecialPopup);
	Q_UNUSED(q)
	if (mPopupButtons.size() == 0) {
        for (int i = 0; i < mNumPopupButtons; ++i) {
			HbTouchKeypadButton *button = new HbTouchKeypadButton(0,QString(""),0);
            q->connect(button, SIGNAL(clicked()), mClickMapper, SLOT(map()));
            mClickMapper->setMapping(button, i);
            mPopupButtons.append(button);
            button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbPopupPortraitButtonTextLayout);
        }

        for (int i = 0; i < mNumPopupButtons; ++i) {
			if(Qt::Key_Asterisk == mButtonId) {
				//This check is for logical separation between Above below vowels , Diacritics and Tone marks 
				//In Thai Language we have 7 Above and Below Vowels, 5 Diacritics and 4 Tone marks
				if(!(i == HBPopupEmptyButtonSeven || i== HBPopupEmptyButtonEight || i == HBPopupEmptyButtonNine)) {		
					mButtonLayout->addItem(mPopupButtons.at(i), i/mPopupGridColumns, i%mPopupGridColumns);
				}
			} else if (Qt::Key_Shift == mButtonId) {
				mButtonLayout->addItem(mPopupButtons.at(i), i/mPopupGridColumns, i%mPopupGridColumns);
			}
        }
    }
	//Set the Layout Dimensions
	setLayoutDimensions(QSizeF(screenWidth/mPopupGridColumns, screenHeight/mPopupGridRows));
	//Assign button layout to widget
	mButtonWidget->setLayout(mButtonLayout);	
}

/*!
This function defines the layout porperties for popup.
*/
void HbInputThaiSpecialPopupPrivate::setLayoutDimensions(QSizeF dimensions)
{
    // only update the dimensions if they are not previously set
    if (mSize == dimensions) {
        return;
    }
    mSize = dimensions;

    mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);

    for (int i = 0; i < mPopupGridColumns; i++) {
        mButtonLayout->setColumnFixedWidth(i, dimensions.width());
    }
    for (int i = 0; i < mPopupGridRows; i++) {
        mButtonLayout->setRowFixedHeight(i, dimensions.height());
    }

    mButtonLayout->setHorizontalSpacing(0.0);
    mButtonLayout->setVerticalSpacing(0.0);
	foreach (HbTouchKeypadButton* button, mPopupButtons) {
        if (button) {
            button->setInitialSize(dimensions);
        }
    }
}


/*!
Gets the special character sets from set keymapping.
*/
void HbInputThaiSpecialPopupPrivate::getSpecialCharacters()
{
    mSpecialCharacterSet.clear();
    if (mKeymap) {
		const HbKeyboardMap* keyboardMap = 0;
		//Set keyboard map depending on Key_Asterisk and Key_Shift
		if(Qt::Key_Asterisk == mButtonId) {
			keyboardMap = mKeymap->keyboard(HbKeyboardThaiStarSctPortrait);
		} else if(Qt::Key_Shift == mButtonId) {
			keyboardMap = mKeymap->keyboard(HbKeyboardThaiHashSctPortrait);
		}
        if (keyboardMap) {
            foreach (const HbMappedKey* mappedKey, keyboardMap->keys) {
				//Creats character set through keyboard map
                mSpecialCharacterSet.append(mappedKey->characters(HbModifierNone));
            }
        }
    }
}

/*!
Let's set Thai Special Character to Buttons
*/
void HbInputThaiSpecialPopupPrivate::setPopupButtons(const QString &aCharSet)
{
    int i = 0;
    int j = 0;
	for (; i < mPopupButtons.size()-1 && j < aCharSet.size(); ++i) {
		if(Qt::Key_Asterisk == mButtonId) {
			//This check is for logical separation between Above below Vowels , Diacritics and Tone marks 
			//In Thai Language we have 7 Above and Below Vowels, 5 Diacritics and 4 Tone marks
			if(!(i == HBPopupEmptyButtonSeven || i== HBPopupEmptyButtonEight || i == HBPopupEmptyButtonNine)) {
				const QChar &character = aCharSet[j];
				mPopupButtons.at(i)->setText(character);
				mPopupButtons.at(i)->setObjectName("Thai Sct portrait " + QString(character));
				j++;
			}
		} else if (Qt::Key_Shift == mButtonId) {
			const QChar &character = aCharSet[j];
			mPopupButtons.at(i)->setText(character);
			mPopupButtons.at(i)->setObjectName("Thai Sct portrait " + QString(character));
			j++;
		}
	}

    for (; i < mPopupButtons.size()-1; ++i) {       
            mPopupButtons.at(i)->setText("");       
    }
    applyEditorConstraints();
	initializeAbcButton();
}

/*!
Apply editor constraints on buttons
*/
void HbInputThaiSpecialPopupPrivate::applyEditorConstraints()
{	
	if (Qt::Key_Asterisk == mButtonId) {
		//Get Character class
		int glyphType = thaiGlyphType(mPrevChar);
		//Set the rules to the editor to allow or disallow characters 
		switch(glyphType) {
		case HbThaiCharNonThai:
		case HbThaiCharNon:
		case HbThaiCharLV:
		case HbThaiCharFV1:
		case HbThaiCharFV2:
		case HbThaiCharFV3:
		case HbThaiCharBD:
		case HbThaiCharTone:
		case HbThaiCharAD1:
		case HbThaiCharAD2:
		case HbThaiCharAD3:
			for (int i=0; i < mPopupButtons.size()-1; ++i) {					
				mPopupButtons.at(i)->setFade(true);     
			}
			break;
		case HbThaiCharCons:
			for (int i=0; i < mPopupButtons.size()-1; ++i) {					
				mPopupButtons.at(i)->setFade(false);     
			}
			break;
		case HbThaiCharAV1:
		case HbThaiCharBV1: {
				static const QChar data[6] = { 0x0e48, 0x0e49, 0x0e4a, 0x0e4b, 0x0e4c, 0x0e4d };
				QString allowChar(data, 6);

				for (int i=0; i < mPopupButtons.size()-1; ++i) { 				
					QString buttonText = mPopupButtons.at(i)->text();

					if(buttonText.isEmpty()) {
						mPopupButtons.at(i)->setFade(true);
					} else {
						if (allowChar.contains(buttonText)) {
							mPopupButtons.at(i)->setFade(false);
						} else {
							mPopupButtons.at(i)->setFade(true);						
						}
					}
				}
			}
			break;
		case HbThaiCharAV2:
		case HbThaiCharBV2: {				
				static const QChar data[4] = { 0x0e48, 0x0e49, 0x0e4a, 0x0e4b };
				QString allowChar(data, 4);

				for (int i=0; i < mPopupButtons.size()-1; ++i) { 				
					QString buttonText = mPopupButtons.at(i)->text();

					if(buttonText.isEmpty()) {
						mPopupButtons.at(i)->setFade(true);
					} else {
						if (allowChar.contains(buttonText)) {
							mPopupButtons.at(i)->setFade(false);
						} else {
							mPopupButtons.at(i)->setFade(true);						
						}
					}

				}
			
			}
			break;

		case HbThaiCharAV3: {				
				static const QChar data[5] = { 0x0e48, 0x0e49, 0x0e4a, 0x0e4b, 0x0e47 };
				QString allowChar(data, 5);

				for (int i=0; i < mPopupButtons.size()-1; ++i) { 				
					QString buttonText = mPopupButtons.at(i)->text();

					if(buttonText.isEmpty()) {
						mPopupButtons.at(i)->setFade(true);
					} else {
						if (allowChar.contains(buttonText)) {
							mPopupButtons.at(i)->setFade(false);
						} else {
							mPopupButtons.at(i)->setFade(true);						
						}
					}
				}
			
			}
			break;
		default:
			break;

		}
	}

}
/*!
Returns Character classes depending on the previous entered character
*/
int HbInputThaiSpecialPopupPrivate::thaiGlyphType(uint prevChar)
{
	if (prevChar >= 0x0E01 && prevChar <= 0x0E2E && prevChar != 0x0E24 && prevChar != 0x0E26 ) {
		return(HbThaiCharCons);
	} else if (prevChar >= 0x0E40 && prevChar <= 0x0E44) {
		return(HbThaiCharLV);
	} else if (prevChar == 0x0E30 || prevChar == 0x0E32 || prevChar == 0x0E33) {
		return(HbThaiCharFV1);
	} else if (prevChar == 0x0E45) {
		return(HbThaiCharFV2);
	} else if (prevChar == 0x0E24 || prevChar == 0x0E26) {
		return(HbThaiCharFV3);
	} else if (prevChar == 0x0E38) {
		return(HbThaiCharBV1);
	} else if (prevChar == 0x0E39) {
		return(HbThaiCharBV2);
	} else if (prevChar == 0x0E3A) {
		return(HbThaiCharBD);
	} else if (prevChar >= 0x0E48 && prevChar <= 0x0E4B) {
		return(HbThaiCharTone);
	} else if (prevChar == 0x0E4C || prevChar == 0x0E4D) {
		return(HbThaiCharAD1);
	} else if (prevChar == 0x0E47) {
		return(HbThaiCharAD2);
	} else if (prevChar == 0x0E4E) {
		return(HbThaiCharAD3);
	} else if (prevChar == 0x0E34) {
		return(HbThaiCharAV1);
	} else if (prevChar == 0x0E31 || prevChar == 0x0E36) {
		return(HbThaiCharAV2);
	} else if (prevChar == 0x0E35 || prevChar == 0x0E37) {
		return(HbThaiCharAV3);
	} else {
		return(HbThaiCharNonThai);
	}
}

/*!
Initialize ABC button
*/
void HbInputThaiSpecialPopupPrivate::initializeAbcButton()
{
	mPopupButtons.at(mAbcButtonId)->setIcon(HbIcon("qtg_mono_alpha_mode"));
    mPopupButtons.at(mAbcButtonId)->setObjectName(HbAbcButtonObjName); 
    mPopupButtons.at(mAbcButtonId)->setObjectName(HbAbcButtonObjName); 
    mPopupButtons.at(mAbcButtonId)->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbPopupPortraitButtonIconLayout);
	mPopupButtons.at(mAbcButtonId)->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
    mPopupButtons.at(mAbcButtonId)->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);    
}


/*!
Handles button clicks.
*/
void HbInputThaiSpecialPopupPrivate::handleStandardButtonClick(int buttonId)
{
   Q_Q(HbInputThaiSpecialPopup);

   if (buttonId >= 0 && buttonId < mNumPopupButtons-1 ) {
	   QString buttonText = mPopupButtons.at(buttonId)->text();
		//Emit the signal when button is not faded and it has some text
	   if (!mPopupButtons.at(buttonId)->isFaded() && buttonText.length() > 0) {
		   emit q->chrSelected(buttonText.at(0));
	   }
   }
   //It will hide popup for any click event
   q->hide();

}

/*!
Handles virtual key clicks
*/
void HbInputThaiSpecialPopupPrivate::_q_mappedKeyClick(int buttonid)
{
    handleStandardButtonClick(buttonid);
}

/// @endcond


/*!
Constructs the object. 
*/

HbInputThaiSpecialPopup::HbInputThaiSpecialPopup(int buttonId, uint prevChar, QGraphicsItem* parent)
    : HbDialog(*new HbInputThaiSpecialPopupPrivate(), parent)
{
    Q_D(HbInputThaiSpecialPopup);
	//Initialize member variable
	d->mPrevChar = prevChar;
	d->mButtonId = buttonId;
#if QT_VERSION >= 0x040600
    // Make sure the Thai special popup never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);
#endif
	// set dialog properties
    setFocusPolicy(Qt::ClickFocus);
    setBackgroundFaded(false);
	setDismissPolicy(TapAnywhere);
    setTimeout(NoTimeout);
	
	d->mButtonLayout = new QGraphicsGridLayout();
    d->mButtonLayout->setSpacing(0.0);
    d->mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
	
	d->mButtonWidget =  new QGraphicsWidget();
	//Create Signal mapper
	d->mClickMapper = new QSignalMapper(this);

	// let's connect buttons to handle click events  
    connect(d->mClickMapper, SIGNAL(mapped(int)), this, SLOT(_q_mappedKeyClick(int)));
}

/*!
Destructs the object.
*/
HbInputThaiSpecialPopup::~HbInputThaiSpecialPopup()
{
}

/*!
This function should be called when ever there is a Key_Asterisk and Key_Shift click happens.
This create buttons, Set the layout dimensions and gets the special characters from the given keymappings and set it accordingly.
*/
void HbInputThaiSpecialPopup::setPopupLayout(const HbKeymap* keymap, uint prevChar, int buttonId, int screenWidth, int screenHeight)
{
    Q_D(HbInputThaiSpecialPopup);
	if(d->mButtonId != buttonId) {
		d->mButtonId = buttonId;
		while (!d->mPopupButtons.isEmpty())
			delete d->mPopupButtons.takeFirst();
	}
	d->mKeymap = keymap;
	d->mPrevChar = prevChar;	
	//Initialize Number of keys dependent on Key_Asterisk and Key_Shift
	d->setNumberOfKeys();
	//Create buttons.
	d->createPopupButtons(screenWidth,screenHeight);
	//Gets the special character sets from set keymapping.
    d->getSpecialCharacters();
	//Let's set Special Character Table Buttons
	d->setPopupButtons(d->mSpecialCharacterSet);
	setContentWidget(d->mButtonWidget);
}

/*!
This a virtual functions in QGraphicsWidget. It is called whenever the Thai popup is shown. 

*/
void HbInputThaiSpecialPopup::showEvent( QShowEvent * event )
{
	HbDialog::showEvent(event);  
}

#include "moc_hbinputthaispecialpopup.cpp"

//End of file

