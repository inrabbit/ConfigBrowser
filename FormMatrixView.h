#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

class UnivType;

namespace ConfigBrowser {

	/// <summary>
	/// FormMatrixView �̊T�v
	///
	/// �x��: ���̃N���X�̖��O��ύX����ꍇ�A���̃N���X���ˑ����邷�ׂĂ� .resx �t�@�C���Ɋ֘A�t����ꂽ
	///          �}�l�[�W ���\�[�X �R���p�C�� �c�[���ɑ΂��� 'Resource File Name' �v���p�e�B��
	///          �ύX����K�v������܂��B���̕ύX���s��Ȃ��ƁA
	///          �f�U�C�i�ƁA���̃t�H�[���Ɋ֘A�t����ꂽ���[�J���C�Y�ς݃��\�[�X�Ƃ��A
	///          ���������݂ɗ��p�ł��Ȃ��Ȃ�܂��B
	/// </summary>
	public ref class FormMatrixView : public System::Windows::Forms::Form
	{
	public:
		FormMatrixView(void);

	protected:
		/// <summary>
		/// �g�p���̃��\�[�X�����ׂăN���[���A�b�v���܂��B
		/// </summary>
		~FormMatrixView();
	protected:
		const UnivType *m_pUnivType;
		System::String^ m_Name;

	public:
		void SetName(System::String^ name);
		void SetContents(const UnivType *pUnivType);
		void UpdateUnivTypeList();
	protected:
		void AddUnivTypeItem(int nRow, int nCol, const UnivType *pUnivType);

	private: System::Windows::Forms::DataGridView^  dataGridViewMatrix;
	protected: 

	protected: 

	private:
		/// <summary>
		/// �K�v�ȃf�U�C�i�ϐ��ł��B
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �f�U�C�i �T�|�[�g�ɕK�v�ȃ��\�b�h�ł��B���̃��\�b�h�̓��e��
		/// �R�[�h �G�f�B�^�ŕύX���Ȃ��ł��������B
		/// </summary>
		void InitializeComponent(void)
		{
			this->dataGridViewMatrix = (gcnew System::Windows::Forms::DataGridView());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridViewMatrix))->BeginInit();
			this->SuspendLayout();
			// 
			// dataGridViewMatrix
			// 
			this->dataGridViewMatrix->AutoSizeColumnsMode = System::Windows::Forms::DataGridViewAutoSizeColumnsMode::AllCells;
			this->dataGridViewMatrix->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridViewMatrix->Dock = System::Windows::Forms::DockStyle::Fill;
			this->dataGridViewMatrix->EditMode = System::Windows::Forms::DataGridViewEditMode::EditProgrammatically;
			this->dataGridViewMatrix->Location = System::Drawing::Point(0, 0);
			this->dataGridViewMatrix->Name = L"dataGridViewMatrix";
			this->dataGridViewMatrix->RowTemplate->Height = 21;
			this->dataGridViewMatrix->Size = System::Drawing::Size(465, 312);
			this->dataGridViewMatrix->TabIndex = 0;
			this->dataGridViewMatrix->CellDoubleClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &FormMatrixView::dataGridViewMatrix_CellDoubleClick);
			// 
			// FormMatrixView
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(465, 312);
			this->Controls->Add(this->dataGridViewMatrix);
			this->Name = L"FormMatrixView";
			this->Text = L"FormMatrixView";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridViewMatrix))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void dataGridViewMatrix_CellDoubleClick(System::Object^  sender, System::Windows::Forms::DataGridViewCellEventArgs^  e);
			 
	};
}
