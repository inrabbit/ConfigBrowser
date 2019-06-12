#include "StdAfx.h"
#include "FormInput.h"
#include "UnivType.h"
#include "CommonDotNet.h"
#include "UpdateManager.h"
#include "BaseDB.h"
#include "RequireUpdateObj.h"

using namespace ConfigBrowser;

#define MAX_STRING 256

void FormInput::UpdateDisplay(UnivType *pUnivType)
{
	if(!pUnivType->isValid()){
		radioButtonInvalid->Checked = true;
	}else if(pUnivType->isEmpty()){
		radioButtonEmpty->Checked = true;
	}else if(pUnivType->isBoolean()){
		radioButtonBool->Checked = true;
		radioButtonTrue->Checked = pUnivType->getBool();
		radioButtonFalse->Checked = !pUnivType->getBool();
	}else if(pUnivType->isMatrix()){
		radioButtonMatrix->Checked = true;
		textBoxRow->Text = String::Format("{0}", pUnivType->getNumRow());
		textBoxCol->Text = String::Format("{0}", pUnivType->getNumColumn());
	}else if(pUnivType->isSubset()){
		radioButtonSubset->Checked = true;
	}else if(pUnivType->isEquation()){
		radioButtonEqn->Checked = true;
	}else if(pUnivType->isInteger()){
		radioButtonInt->Checked = true;
		textBoxMain->Text = String::Format("{0}", pUnivType->getInt());
	}else if(pUnivType->isUInteger()){
		radioButtonUnsigned->Checked = true;
		textBoxMain->Text = String::Format("{0}", pUnivType->getUInt());		
	}else if(pUnivType->isReal()){
		radioButtonReal->Checked = true;
		textBoxMain->Text = String::Format("{0}", pUnivType->getReal());
	}else if(pUnivType->isString()){
		radioButtonStr->Checked = true;
		textBoxMain->Text = gcnew String(pUnivType->getString());
	}else if(pUnivType->isBinary()){
		radioButtonBin->Checked = true;
	}
}

System::Void FormInput::OnTypeChanged(System::Object^  sender, System::EventArgs^  e)
{
	groupBoxGen->Enabled = false;
	groupBoxBool->Enabled = false;
	groupBoxMat->Enabled = false;
	if(radioButtonBool->Checked){
		groupBoxBool->Enabled = true;
	}
	else if(radioButtonMatrix->Checked){
		groupBoxMat->Enabled = true;
		if(textBoxCol->Text->Length == 0) textBoxCol->Text = "1";
		if(textBoxRow->Text->Length == 0) textBoxRow->Text = "1";
		textBoxRow->Focus();
		textBoxRow->SelectAll();
	}
	else if(!radioButtonInvalid->Checked && !radioButtonEmpty->Checked){
		groupBoxGen->Enabled = true;
		textBoxMain->Focus();
		textBoxMain->SelectAll();
	}
}

System::Void FormInput::buttonOK_Click(System::Object^  sender, System::EventArgs^  e)
{
	UpdateKind kind = UK_ONLY_VALUE;

	if(radioButtonInvalid->Checked){
		m_pUnivType->invalidate();
	}else if(radioButtonEmpty->Checked){
		m_pUnivType->clear();
	}else if(radioButtonBool->Checked){
		m_pUnivType->setBool(radioButtonTrue->Checked);
	}else if(radioButtonMatrix->Checked){
		m_pUnivType->setMatrix(Convert::ToInt16(textBoxRow->Text), Convert::ToInt16(textBoxCol->Text));
		kind = UK_ONLY_ARRAY_SIZE;
	}else if(radioButtonSubset->Checked){
		// 現状、Subsetは動的に設定できない
		//m_pUnivType->setSubset(new BaseDB);
	}else if(radioButtonEqn->Checked){
		//
	}else if(radioButtonInt->Checked){
		m_pUnivType->setInt(Convert::ToInt32(textBoxMain->Text));
	}else if(radioButtonUnsigned->Checked){
		m_pUnivType->setUInt(Convert::ToUInt32(textBoxMain->Text));
	}else if(radioButtonReal->Checked){
		m_pUnivType->setReal(Convert::ToSingle(textBoxMain->Text));
	}else if(radioButtonStr->Checked){
		std::string text = from_cli(textBoxMain->Text);
		m_pUnivType->setString(text.c_str());
	}else if(radioButtonBin->Checked){
		//
	}

	// DBに更新を通知する
	RequireUpdateObj::getDB()->issueNotifyUpdated(m_pUnivType, kind);
	// 表示ウィンドウを更新する
	UpdateManager::NotifyUpdate();
	Close();
}
