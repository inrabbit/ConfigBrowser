#include "StdAfx.h"
#include "FormMatrixView.h"
#include "CommonDotNet.h"
#include "BaseDB.h"
#include "NamedUnivType.h"
#include <cassert>
#include "FormUnivTypeList.h"
#include "FormInput.h"
#include "UpdateManager.h"

using namespace ConfigBrowser;

#define UNIVTYPE_ITEM_VALUE  0
#define UNIVTYPE_ITEM_SUBSET 1
#define UNIVTYPE_ITEM_ARRAY  2

FormMatrixView::FormMatrixView(void)
{
	InitializeComponent();
	//
	//TODO: ここにコンストラクタ コードを追加します
	//
	UpdateManager::AddToUpdateList(WT_FORM_MATRIX_VIEW, this);
	m_pUnivType = NULL;
}

FormMatrixView::~FormMatrixView()
{
	if (components)
	{
		delete components;
	}
	UpdateManager::RemoveFromUpdateList(this);
}

void FormMatrixView::SetName(System::String^ name)
{
	m_Name = name;
	Text = String::Concat("ARRAY - ", name);
}

void FormMatrixView::SetContents(const UnivType *pUnivType)
{
	m_pUnivType = pUnivType;

	try{
		UpdateUnivTypeList();
	}
	catch(CdbException& e){
		MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
	}
	catch(...){
		MessageBox::Show("Unknown Error", "ERROR"); 
	}
}

void FormMatrixView::UpdateUnivTypeList()
{
	if(m_pUnivType->isSubset()){
		// 更新された結果、表示できなくなった場合はウィンドウを閉じる
		Close();
	}

	dataGridViewMatrix->Columns->Clear();
	dataGridViewMatrix->Rows->Clear();

	int nRow = m_pUnivType->getNumRow();
	int nCol = m_pUnivType->getNumColumn();

	int i, j;
	for(i = 0; i < nCol; i++){
		System::String^ text = i.ToString();
		dataGridViewMatrix->Columns->Add(text, text);
	}
	for(i = 0; i < nRow; i++){
		dataGridViewMatrix->Rows->Add();
	}
	for(i = 0; i < nRow; i++){
		dataGridViewMatrix->Rows[i]->HeaderCell->Value = i.ToString();
	}
	for(i = 0; i < nRow; i++){
		for(j = 0; j < nCol; j++){
			AddUnivTypeItem(i, j, &m_pUnivType->getAtConst(i, j));
		}
	}
}

void FormMatrixView::AddUnivTypeItem(int nRow, int nCol, const UnivType *pUnivType)
{
	// タグ
	int tag = UNIVTYPE_ITEM_VALUE;

	// 値の配色
	System::Drawing::Color foreColor = System::Drawing::SystemColors::WindowText;
	System::Drawing::Color backColor = System::Drawing::SystemColors::Window;

	// 値のフォーマット文字列
	System::String^ Value;
	if(!pUnivType->isValid()){
		Value = "INVALID";
		foreColor = System::Drawing::Color::White;
		backColor = System::Drawing::Color::Black;
	}else if(pUnivType->isEmpty()){
		Value = "EMPTY";
		foreColor = System::Drawing::Color::White;
		backColor = System::Drawing::Color::Black;
	}else if(pUnivType->isMatrix()){
		Value = String::Format("MATRIX ({0}, {1})", pUnivType->getNumRow(), pUnivType->getNumColumn());
		foreColor = System::Drawing::Color::White;
		backColor = System::Drawing::Color::Red;
		tag = UNIVTYPE_ITEM_ARRAY;
	}else{
		assert(pUnivType->isScalar());

		if(pUnivType->isInteger()){
			Value = String::Format("{0}", pUnivType->getInt());
		}else if(pUnivType->isUInteger()){
			Value = String::Format("{0} UL", pUnivType->getUInt());
		}else if(pUnivType->isReal()){
			Value = String::Format("{0}", pUnivType->getReal());
		}else if(pUnivType->isPointer()){
			Value = String::Format("{0:0x8}", (unsigned int)pUnivType->getPointer());
		}else if(pUnivType->isBoolean()){
			Value = pUnivType->getBool() ? "true" : "false";
			foreColor = System::Drawing::Color::White;
			backColor = System::Drawing::Color::Magenta;
		}else if(pUnivType->isString()){
			Value = gcnew String(pUnivType->getString());
			foreColor = System::Drawing::Color::Black;
			backColor = System::Drawing::Color::Yellow;
		}else if(pUnivType->isBinary()){
			Value = String::Format("BINARY ({0} bytes)", pUnivType->getBinarySize());
			foreColor = System::Drawing::Color::White;
			backColor = System::Drawing::Color::Black;
		}else if(pUnivType->isSubset()){
			Value = "SUBSET";
			foreColor = System::Drawing::Color::Black;
			backColor = System::Drawing::Color::Orange;
			tag = UNIVTYPE_ITEM_SUBSET;
		}
	}

	dataGridViewMatrix[nCol, nRow]->Value = Value;
	dataGridViewMatrix[nCol, nRow]->Style->ForeColor = foreColor;
	dataGridViewMatrix[nCol, nRow]->Style->BackColor = backColor;
	dataGridViewMatrix[nCol, nRow]->Tag = tag;
}

System::Void FormMatrixView::dataGridViewMatrix_CellDoubleClick(System::Object^  sender, System::Windows::Forms::DataGridViewCellEventArgs^  e)
{
	// 選択された要素をDBから取得
	int nRow = e->RowIndex;
	int nCol = e->ColumnIndex;
	const UnivType *pSelectedItem;
	try{
		pSelectedItem = pSelectedItem = &m_pUnivType->getAtConst(nRow, nCol);
	}
	catch(CdbException& e){
		MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
		return;
	}
	catch(...){
		MessageBox::Show("Unknown Error", "ERROR"); 
		return;
	}

	int tag = (int)dataGridViewMatrix[nCol, nRow]->Tag;

	switch(tag){
	case UNIVTYPE_ITEM_SUBSET:
		{
			FormUnivTypeList^ form = gcnew FormUnivTypeList;
			form->SetContents(pSelectedItem);
			form->Text = String::Concat("SUBSET - ", m_Name, "(", nRow.ToString(), ", ", nCol.ToString(), ")");
			form->Show();
		}
		break;
	case UNIVTYPE_ITEM_ARRAY:
		{
			FormMatrixView^ form = gcnew FormMatrixView;
			form->SetContents(pSelectedItem);
			form->SetName(String::Concat(m_Name, "(", nRow.ToString(), ", ", nCol.ToString(), ")"));
			form->Show();
		}
		break;
	default:
		if(!pSelectedItem->isConstant()){
			FormInput^ form = gcnew FormInput(const_cast<UnivType *>(pSelectedItem));
			form->Text = String::Concat("EDIT - ", m_Name, "(", nRow.ToString(), ", ", nCol.ToString(), ")");
			form->Show();
		}
	}

}
