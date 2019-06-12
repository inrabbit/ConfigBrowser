#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

class UnivType;
class BaseDB;

namespace ConfigBrowser {

	/// <summary>
	/// FormUnivTypeList �̊T�v
	///
	/// �x��: ���̃N���X�̖��O��ύX����ꍇ�A���̃N���X���ˑ����邷�ׂĂ� .resx �t�@�C���Ɋ֘A�t����ꂽ
	///          �}�l�[�W ���\�[�X �R���p�C�� �c�[���ɑ΂��� 'Resource File Name' �v���p�e�B��
	///          �ύX����K�v������܂��B���̕ύX���s��Ȃ��ƁA
	///          �f�U�C�i�ƁA���̃t�H�[���Ɋ֘A�t����ꂽ���[�J���C�Y�ς݃��\�[�X�Ƃ��A
	///          ���������݂ɗ��p�ł��Ȃ��Ȃ�܂��B
	/// </summary>
	public ref class FormUnivTypeList : public System::Windows::Forms::Form
	{
	public:
		FormUnivTypeList();
	protected:
		BaseDB *m_pDB;
		const UnivType *m_pUnivType;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  outputStructureOfTheItemToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  outputStructureOfAllItemsToolStripMenuItem;
	protected: 

			 bool m_IsRequireDeleteContents;

	public:
		void SetContents(const UnivType *pUnivType);
		void SetFileName(System::String^ FileName);
		void Clear();
		void UpdateUnivTypeList();
	protected:
		void AddUnivTypeItem(System::String^ Name, const UnivType *pUnivType);
		void LoadParamFileTop(System::String^ FileName, BaseDB *pDB);
		void LoadParamFileSingle(System::String^ FileName, BaseDB *pDB);

	protected:
		/// <summary>
		/// �g�p���̃��\�[�X�����ׂăN���[���A�b�v���܂��B
		/// </summary>
		~FormUnivTypeList();

	private: System::Windows::Forms::ListView^  UnivTypeList;
	private: System::Windows::Forms::ColumnHeader^  columnHeaderName;
	private: System::Windows::Forms::ColumnHeader^  columnHeaderVal;
	private: System::ComponentModel::IContainer^  components;
	protected: 

	protected: 

	private:
		/// <summary>
		/// �K�v�ȃf�U�C�i�ϐ��ł��B
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// �f�U�C�i �T�|�[�g�ɕK�v�ȃ��\�b�h�ł��B���̃��\�b�h�̓��e��
		/// �R�[�h �G�f�B�^�ŕύX���Ȃ��ł��������B
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->UnivTypeList = (gcnew System::Windows::Forms::ListView());
			this->columnHeaderName = (gcnew System::Windows::Forms::ColumnHeader());
			this->columnHeaderVal = (gcnew System::Windows::Forms::ColumnHeader());
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->outputStructureOfTheItemToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->outputStructureOfAllItemsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->contextMenuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// UnivTypeList
			// 
			this->UnivTypeList->AllowDrop = true;
			this->UnivTypeList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->columnHeaderName, 
				this->columnHeaderVal});
			this->UnivTypeList->ContextMenuStrip = this->contextMenuStrip1;
			this->UnivTypeList->Dock = System::Windows::Forms::DockStyle::Fill;
			this->UnivTypeList->FullRowSelect = true;
			this->UnivTypeList->Location = System::Drawing::Point(0, 0);
			this->UnivTypeList->Name = L"UnivTypeList";
			this->UnivTypeList->Size = System::Drawing::Size(304, 489);
			this->UnivTypeList->TabIndex = 0;
			this->UnivTypeList->UseCompatibleStateImageBehavior = false;
			this->UnivTypeList->View = System::Windows::Forms::View::Details;
			this->UnivTypeList->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &FormUnivTypeList::UnivTypeList_DragDrop);
			this->UnivTypeList->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &FormUnivTypeList::UnivTypeList_DragEnter);
			this->UnivTypeList->DoubleClick += gcnew System::EventHandler(this, &FormUnivTypeList::UnivTypeList_DoubleClick);
			this->UnivTypeList->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &FormUnivTypeList::UnivTypeList_MouseUp);
			// 
			// columnHeaderName
			// 
			this->columnHeaderName->Text = L"Name";
			this->columnHeaderName->Width = 140;
			// 
			// columnHeaderVal
			// 
			this->columnHeaderVal->Text = L"Value";
			this->columnHeaderVal->Width = 140;
			// 
			// contextMenuStrip1
			// 
			this->contextMenuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->outputStructureOfTheItemToolStripMenuItem, 
				this->outputStructureOfAllItemsToolStripMenuItem});
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->Size = System::Drawing::Size(243, 48);
			// 
			// outputStructureOfTheItemToolStripMenuItem
			// 
			this->outputStructureOfTheItemToolStripMenuItem->Name = L"outputStructureOfTheItemToolStripMenuItem";
			this->outputStructureOfTheItemToolStripMenuItem->Size = System::Drawing::Size(242, 22);
			this->outputStructureOfTheItemToolStripMenuItem->Text = L"Output structure of the item";
			this->outputStructureOfTheItemToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormUnivTypeList::outputStructureOfTheItemToolStripMenuItem_Click);
			// 
			// outputStructureOfAllItemsToolStripMenuItem
			// 
			this->outputStructureOfAllItemsToolStripMenuItem->Name = L"outputStructureOfAllItemsToolStripMenuItem";
			this->outputStructureOfAllItemsToolStripMenuItem->Size = System::Drawing::Size(242, 22);
			this->outputStructureOfAllItemsToolStripMenuItem->Text = L"Output structure of all items";
			this->outputStructureOfAllItemsToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormUnivTypeList::outputStructureOfAllItemsToolStripMenuItem_Click);
			// 
			// FormUnivTypeList
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(304, 489);
			this->Controls->Add(this->UnivTypeList);
			this->Name = L"FormUnivTypeList";
			this->Text = L"FormUnivTypeList";
			this->contextMenuStrip1->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void UnivTypeList_DoubleClick(System::Object^  sender, System::EventArgs^  e);
	private: System::Void UnivTypeList_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
	private: System::Void UnivTypeList_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
	private: System::Void UnivTypeList_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	private: System::Void outputStructureOfTheItemToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void outputStructureOfAllItemsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
};
}
