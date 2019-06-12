#include "StdAfx.h"
#include "FormUnivTypeList.h"
#include "CommonDotNet.h"
#include "BaseDB.h"
#include "NamedUnivType.h"
#include <cassert>
#include "FormInput.h"
#include "UpdateManager.h"
#include "TokenAnaly.h"
#include "BaseDB.h"
#include "Parser.h"
#include "RequireUpdateObj.h"
#include "CommonLib.h"
#include "CommonDotNet.h"
#include "FormMatrixView.h"

using namespace ConfigBrowser;

#define UNIVTYPE_ITEM_VALUE  0
#define UNIVTYPE_ITEM_SUBSET 1
#define UNIVTYPE_ITEM_ARRAY  2

FormUnivTypeList::FormUnivTypeList() : m_pDB(NULL), m_pUnivType(NULL), m_IsRequireDeleteContents(false)
{
	InitializeComponent();
	//
	//TODO: �����ɃR���X�g���N�^ �R�[�h��ǉ����܂�
	//
	UpdateManager::AddToUpdateList(WT_FORM_UNIVTYPE_LIST, this);
}

FormUnivTypeList::~FormUnivTypeList()
{
	if (components)
	{
		delete components;
	}
	UpdateManager::RemoveFromUpdateList(this);
	Clear();
}

void FormUnivTypeList::Clear()
{
	try{
		if(m_IsRequireDeleteContents && (m_pUnivType != NULL)){
			delete m_pUnivType;
			m_pUnivType = NULL;
		}
	}
	catch(CdbException& e){
		MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
	}
	catch(System::Exception^ e){
		MessageBox::Show(e->Message->ToString(), "Exception");
	}
	catch(...){
		MessageBox::Show("Unknown Error", "ERROR"); 
	}
}

void FormUnivTypeList::SetContents(const UnivType *pUnivType)
{
	if(pUnivType == NULL) return;
	Clear();
	m_pUnivType = pUnivType;
	UpdateUnivTypeList();
}

void FormUnivTypeList::SetFileName(System::String^ FileName)
{
	if(FileName == nullptr) return;
	Clear();
	Text = FileName;
	clib_set_default_path(from_cli(FileName).c_str());

	// �t�@�C������J���ꍇ�ɂ́AUnivType�Ƀ_�~�[��ݒ�
	UnivType *pUnivType = new UnivType;
	m_pUnivType = pUnivType;

	try{
		// create database object
		m_pDB = new BaseDB;
		m_IsRequireDeleteContents = true;	// m_pUnivType���E�B���h�E�������ۂɉ������
		pUnivType->setSubset(m_pDB);	// m_pUnivType���폜�����ۂ�m_pDB���폜�����

		LoadParamFileTop(FileName, m_pDB);

		m_pDB->update();

		// �\���̍X�V
		UpdateUnivTypeList();
	}
	catch(CdbException& e){
		MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
	}
	catch(System::NullReferenceException^ e){
		MessageBox::Show(e->Message->ToString(), "NullReferenceException");
	}
	catch(System::Exception^ e){
		MessageBox::Show(e->Message->ToString(), "Exception");
	}
	catch(...){
		MessageBox::Show("Unknown Error", "ERROR"); 
	}
}

#define MAX_INCLUDES 50
void FormUnivTypeList::LoadParamFileTop(System::String^ FileName, BaseDB *pDB)
{
	/*
	 * �t�@�C���p�X���� 
	 * �ő�œ����ɓǂݍ��߂�t�@�C��������path�̈���m�ۂ��Ă���
	 */
	cli::array<System::String^>^ IncludeFiles = gcnew cli::array<System::String^>(MAX_INCLUDES);

	int max_includes = 1;	// �ǂݍ��݂��K�v�ȃt�@�C���� 

	// �g�b�v���x���̃t�@�C�������R�s�[
	IncludeFiles[0] = FileName;

	int done_includes = 0;	// �ǂݍ��݊��������t�@�C���� 
	int i;

	// �R���t�B�O�t�@�C�����}�[�W���[�h 
	for(done_includes = 0; done_includes < max_includes; done_includes++){
		// �t�@�C�����P�ǂݍ��� 
		LoadParamFileSingle(IncludeFiles[done_includes], pDB);

		// INCLUDE�L�[�����邩�ǂ����`�F�b�N���� 
		static const char *pszIncludeLabel = "INCLUDE";

		CDB_ERR error;
		const UnivType& IncludeList = pDB->getConstUnivType(pszIncludeLabel, error);
		if(error == CDB_OK){
			int nIncludes = IncludeList.getNumElements();
			for(i = 0; i < nIncludes; i++){
				IncludeFiles[max_includes] = to_cli(std::string((const char *)IncludeList.getAtConst(i)));

				// ���ɓ���̃t�@�C�������������ꍇ�ɂ̓��X�g����폜���� 
				int j;
				for(j = 0; j < max_includes; j++){
					if(IncludeFiles[max_includes] == IncludeFiles[j]){
						break;
					}
				}
				if(j == max_includes){
					max_includes++;
					if((max_includes + 1) > MAX_INCLUDES){
						MessageBox::Show("number of include files exceeded");	
					}
				}
			}
			// INCLUDE�L�[���폜���� 
			pDB->remove(pszIncludeLabel);
		}
	}
}

void FormUnivTypeList::LoadParamFileSingle(System::String^ FileName, BaseDB *pDB)
{
	if(FileName == nullptr) return;

	// read script file
	std::string StdFileName = from_cli(FileName);
	int size = clib_get_file_size(StdFileName.c_str());
	if(size == CLIB_ERROR_FOPEN){
		MessageBox::Show(System::String::Concat("Cannot open the file - ", FileName), "ERROR"); 
		return;
	}
	char *pText = new char[size + 1];
	clib_file_read(StdFileName.c_str(), pText, size + 1);
	pText[size] = '\0';

	try{
		// create token object
		TokenAnaly token(pText);

		// set container for equation objects
		// �y���ӁzParser�𓮂����O�ɁA�K��������s������
		RequireUpdateObj::setDB(pDB);

		// create parser object
		Parser parser(token, *pDB);

		parser.execute();
	}
	catch(CdbException& e){
		e.addFileName(StdFileName.c_str());
		MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
	}
	catch(System::NullReferenceException^ e){
		MessageBox::Show(e->Message->ToString(), "NullReferenceException");
	}
	catch(System::Exception^ e){
		MessageBox::Show(e->Message->ToString(), "Exception");
	}
	catch(...){
		MessageBox::Show("Unknown Error", "ERROR"); 
	}

	delete[] pText;
}

void FormUnivTypeList::UpdateUnivTypeList()
{
	UnivTypeList->Items->Clear();

	if(m_pUnivType->isSubset()){
		m_pDB = m_pUnivType->getSubsetDB();
	}else{
		m_pDB = NULL;
	}

	if(m_pUnivType->isSubset()){
		WrappedMap *pMap = m_pDB->getContainer();

		// �v�f��
		const NamedUnivType *pUnivType;
		for(pUnivType = pMap->getNext(true); pUnivType != NULL; pUnivType = pMap->getNext()){

			// ���x����
			System::String^ Name = gcnew System::String(pUnivType->getName());
			// ���ڂ̒ǉ�
			AddUnivTypeItem(Name, pUnivType);
		}
	}else{
		int nRow = m_pUnivType->getNumRow();
		int nCol = m_pUnivType->getNumColumn();

		for(int j = 0; j < nCol; j++){
			for(int i = 0; i < nRow; i++){
				const UnivType& entry = m_pUnivType->getAtConst(i, j);
				// ���x����
				System::String^ Name = System::String::Format("{0}, {1}", i, j);
				// ���ڂ̒ǉ�
				AddUnivTypeItem(Name, &entry);
			}
		}

	}
}

void FormUnivTypeList::AddUnivTypeItem(System::String^ Name, const UnivType *pUnivType)
{
	// �^�O
	int tag = UNIVTYPE_ITEM_VALUE;

	// �l�̔z�F
	System::Drawing::Color foreColor = System::Drawing::SystemColors::WindowText;
	System::Drawing::Color backColor = System::Drawing::SystemColors::Window;

	// �l�̃t�H�[�}�b�g������
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

	// �ϐ����̔z�F
	System::Drawing::Color foreColorName = System::Drawing::SystemColors::WindowText;
	System::Drawing::Color backColorName = System::Drawing::SystemColors::Window;
	if(!pUnivType->isConstant()){
		foreColorName = System::Drawing::Color::White;
		backColorName = System::Drawing::Color::Green;
	}

#if 0
	System::Windows::Forms::ListViewItem^  listViewItem = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::Windows::Forms::ListViewItem::ListViewSubItem^  >(2) {(gcnew System::Windows::Forms::ListViewItem::ListViewSubItem(nullptr, 
			Name, System::Drawing::SystemColors::WindowText, System::Drawing::SystemColors::Window, (gcnew System::Drawing::Font(L"MS UI Gothic", 
			9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128))))), (gcnew System::Windows::Forms::ListViewItem::ListViewSubItem(nullptr, 
			Value, foreColor, backColor, (gcnew System::Drawing::Font(L"MS UI Gothic", 
			9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)))))}, -1));

	UnivTypeList->Items->AddRange(gcnew cli::array< System::Windows::Forms::ListViewItem^  >(1) {listViewItem});
#endif

	System::Windows::Forms::ListViewItem^ listViewItem = gcnew ListViewItem(Name);
	listViewItem->UseItemStyleForSubItems = false;
	listViewItem->SubItems->Add(Value);
	listViewItem->SubItems[0]->ForeColor = foreColorName;
	listViewItem->SubItems[0]->BackColor = backColorName;
	listViewItem->SubItems[1]->ForeColor = foreColor;
	listViewItem->SubItems[1]->BackColor = backColor;
	listViewItem->Tag = tag;
	UnivTypeList->Items->Add(listViewItem);
}

System::Void FormUnivTypeList::UnivTypeList_DoubleClick(System::Object^  sender, System::EventArgs^  e) 
{
	int tag = (int)UnivTypeList->SelectedItems[0]->Tag;
	std::string name = from_cli(UnivTypeList->SelectedItems[0]->Text);

	// �I�����ꂽ�v�f��DB����擾
	const UnivType *pSelectedItem = NULL;
	if(m_pUnivType->isSubset()){
		pSelectedItem = &m_pDB->getConstUnivType(name.c_str());
	}else{
		//String^ index = UnivTypeList->SelectedItems[0]->Text->Split(gcnew array<wchar_t>(2){',', ' '});
		array<String^>^ index = UnivTypeList->SelectedItems[0]->Text->Split(',');
		int row = Convert::ToInt32(index[0]);
		int col = Convert::ToInt32(index[1]);
		pSelectedItem = &m_pUnivType->getAt(row, col);
	}

	switch(tag){
	case UNIVTYPE_ITEM_SUBSET:
		{
			FormUnivTypeList^ form = gcnew FormUnivTypeList;
			form->SetContents(pSelectedItem);
			form->Text = String::Concat("SUBSET - ", UnivTypeList->SelectedItems[0]->Text);
			form->Show();
		}
		break;
	case UNIVTYPE_ITEM_ARRAY:
		{
			FormMatrixView^ form = gcnew FormMatrixView;
			form->SetContents(pSelectedItem);
			form->SetName(UnivTypeList->SelectedItems[0]->Text);
			form->Show();
		}
		break;
	default:
		if(!pSelectedItem->isConstant()){
			FormInput^ form = gcnew FormInput(const_cast<UnivType *>(pSelectedItem));
			form->Text = String::Concat("EDIT - ", UnivTypeList->SelectedItems[0]->Text);
			form->Show();
		}
	}
}

System::Void FormUnivTypeList::UnivTypeList_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if(UnivTypeList->SelectedItems == nullptr || UnivTypeList->SelectedItems->Count != 0){
		this->outputStructureOfTheItemToolStripMenuItem->Text = L"Output structure of the item '" + UnivTypeList->SelectedItems[0]->Text + "'";
		this->outputStructureOfTheItemToolStripMenuItem->Enabled = true;
	}else{
		this->outputStructureOfTheItemToolStripMenuItem->Text = L"Output structure of the item";
		this->outputStructureOfTheItemToolStripMenuItem->Enabled = false;
	}
	outputStructureOfAllItemsToolStripMenuItem->Enabled = (m_pDB != NULL);
}

System::Void FormUnivTypeList::outputStructureOfTheItemToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	System::Windows::Forms::SaveFileDialog^ sfd = gcnew System::Windows::Forms::SaveFileDialog;
	sfd->Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
	sfd->FilterIndex = 1;
	sfd->DefaultExt = "txt";
	sfd->RestoreDirectory = true;
	sfd->FileName = UnivTypeList->SelectedItems[0]->Text;

	if ( sfd->ShowDialog() == ::DialogResult::OK )
	{
		std::string name = from_cli(UnivTypeList->SelectedItems[0]->Text);

		// �I�����ꂽ�v�f��DB����擾
		const UnivType *pSelectedItem = NULL;
		if(m_pUnivType->isSubset()){
			pSelectedItem = &m_pDB->getConstUnivType(name.c_str());
		}else{
			//String^ index = UnivTypeList->SelectedItems[0]->Text->Split(gcnew array<wchar_t>(2){',', ' '});
			array<String^>^ index = UnivTypeList->SelectedItems[0]->Text->Split(',');
			int row = Convert::ToInt32(index[0]);
			int col = Convert::ToInt32(index[1]);
			pSelectedItem = &m_pUnivType->getAt(row, col);
		}

		System::IO::StreamWriter^ sw = gcnew System::IO::StreamWriter(sfd->FileName);
		sw->AutoFlush = true;
		System::IO::TextWriter^ tw = System::IO::TextWriter::Synchronized(sw);
		System::Console::SetOut(tw);

		try{
			pSelectedItem->debugPrintStructure(0, false);
		}
		catch(CdbException& e){
			MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
		}

		tw->Close();

		// Recover the standard output stream
		System::IO::StreamWriter^ standardOutput = gcnew System::IO::StreamWriter( Console::OpenStandardOutput() );
		standardOutput->AutoFlush = true;
		Console::SetOut( standardOutput );
	}
}

System::Void FormUnivTypeList::outputStructureOfAllItemsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	System::Windows::Forms::SaveFileDialog^ sfd = gcnew System::Windows::Forms::SaveFileDialog;
	sfd->Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
	sfd->FilterIndex = 1;
	sfd->DefaultExt = "txt";
	sfd->RestoreDirectory = true;

	if ( sfd->ShowDialog() == ::DialogResult::OK )
	{
		System::IO::StreamWriter^ sw = gcnew System::IO::StreamWriter(sfd->FileName);
		sw->AutoFlush = true;
		System::IO::TextWriter^ tw = System::IO::TextWriter::Synchronized(sw);
		System::Console::SetOut(tw);

		try{
			m_pDB->debugPrintStructure(0);
		}
		catch(CdbException& e){
			MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
		}

		tw->Close();

		// Recover the standard output stream
		System::IO::StreamWriter^ standardOutput = gcnew System::IO::StreamWriter( Console::OpenStandardOutput() );
		standardOutput->AutoFlush = true;
		Console::SetOut( standardOutput );
	}
}

System::Void FormUnivTypeList::UnivTypeList_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
{
	if(e->Data->GetDataPresent(DataFormats::FileDrop)){
		// �h���b�O���ꂽ�f�[�^�`���𒲂ׁA�t�@�C���̂Ƃ��̓R�s�[�Ƃ���
		e->Effect = DragDropEffects::Copy;
	}else{
		// �t�@�C���ȊO�͎󂯕t���Ȃ�
		e->Effect = DragDropEffects::None;
	}
}

System::Void FormUnivTypeList::UnivTypeList_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
{
	if(e->Data->GetDataPresent(DataFormats::FileDrop)){
		array<System::String ^> ^files = (array<System::String ^> ^)e->Data->GetData(DataFormats::FileDrop);
		SetFileName(files[0]);
	}
}
