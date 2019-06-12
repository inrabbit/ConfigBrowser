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
	/// FormInput の概要
	///
	/// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
	///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
	///          変更する必要があります。この変更を行わないと、
	///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
	///          正しく相互に利用できなくなります。
	/// </summary>
	public ref class FormInput : public System::Windows::Forms::Form
	{
	public:
		void UpdateDisplay(UnivType *pUnivType);
	public:
		FormInput(UnivType *pUnivType) : m_pUnivType(pUnivType)
		{
			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//
			UpdateDisplay(pUnivType);
		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~FormInput()
		{
			if (components)
			{
				delete components;
			}
		}
	protected:
		UnivType *m_pUnivType;

	// --------------------------------------------------
	private: System::Windows::Forms::RadioButton^  radioButtonInt;
	protected: 
	private: System::Windows::Forms::RadioButton^  radioButtonUnsigned;
	private: System::Windows::Forms::RadioButton^  radioButtonReal;
	private: System::Windows::Forms::RadioButton^  radioButtonBool;
	private: System::Windows::Forms::RadioButton^  radioButtonBin;
	private: System::Windows::Forms::RadioButton^  radioButtonSubset;
	private: System::Windows::Forms::RadioButton^  radioButtonMatrix;
	private: System::Windows::Forms::RadioButton^  radioButtonEqn;

	private: System::Windows::Forms::Button^  buttonOK;
	private: System::Windows::Forms::TextBox^  textBoxMain;
	private: System::Windows::Forms::TextBox^  textBoxRow;
	private: System::Windows::Forms::TextBox^  textBoxCol;





	private: System::Windows::Forms::GroupBox^  groupBoxBool;
	private: System::Windows::Forms::RadioButton^  radioButtonFalse;
	private: System::Windows::Forms::RadioButton^  radioButtonTrue;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::GroupBox^  groupBoxMat;
	private: System::Windows::Forms::GroupBox^  groupBoxGen;
	private: System::Windows::Forms::RadioButton^  radioButtonStr;
	private: System::Windows::Forms::RadioButton^  radioButtonInvalid;
	private: System::Windows::Forms::RadioButton^  radioButtonEmpty;





	private:
		/// <summary>
		/// 必要なデザイナ変数です。
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナ サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディタで変更しないでください。
		/// </summary>
		void InitializeComponent(void)
		{
			this->radioButtonInt = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonUnsigned = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonReal = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonBool = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonBin = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonSubset = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonMatrix = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonEqn = (gcnew System::Windows::Forms::RadioButton());
			this->buttonOK = (gcnew System::Windows::Forms::Button());
			this->textBoxMain = (gcnew System::Windows::Forms::TextBox());
			this->textBoxRow = (gcnew System::Windows::Forms::TextBox());
			this->textBoxCol = (gcnew System::Windows::Forms::TextBox());
			this->groupBoxBool = (gcnew System::Windows::Forms::GroupBox());
			this->radioButtonFalse = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonTrue = (gcnew System::Windows::Forms::RadioButton());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->groupBoxMat = (gcnew System::Windows::Forms::GroupBox());
			this->groupBoxGen = (gcnew System::Windows::Forms::GroupBox());
			this->radioButtonStr = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonInvalid = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonEmpty = (gcnew System::Windows::Forms::RadioButton());
			this->groupBoxBool->SuspendLayout();
			this->groupBoxMat->SuspendLayout();
			this->groupBoxGen->SuspendLayout();
			this->SuspendLayout();
			// 
			// radioButtonInt
			// 
			this->radioButtonInt->AutoSize = true;
			this->radioButtonInt->Location = System::Drawing::Point(74, 12);
			this->radioButtonInt->Name = L"radioButtonInt";
			this->radioButtonInt->Size = System::Drawing::Size(58, 16);
			this->radioButtonInt->TabIndex = 0;
			this->radioButtonInt->TabStop = true;
			this->radioButtonInt->Text = L"Integer";
			this->radioButtonInt->UseVisualStyleBackColor = true;
			this->radioButtonInt->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonUnsigned
			// 
			this->radioButtonUnsigned->AutoSize = true;
			this->radioButtonUnsigned->Location = System::Drawing::Point(74, 34);
			this->radioButtonUnsigned->Name = L"radioButtonUnsigned";
			this->radioButtonUnsigned->Size = System::Drawing::Size(109, 16);
			this->radioButtonUnsigned->TabIndex = 1;
			this->radioButtonUnsigned->TabStop = true;
			this->radioButtonUnsigned->Text = L"Unsigned Integer";
			this->radioButtonUnsigned->UseVisualStyleBackColor = true;
			this->radioButtonUnsigned->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonReal
			// 
			this->radioButtonReal->AutoSize = true;
			this->radioButtonReal->Location = System::Drawing::Point(74, 56);
			this->radioButtonReal->Name = L"radioButtonReal";
			this->radioButtonReal->Size = System::Drawing::Size(46, 16);
			this->radioButtonReal->TabIndex = 2;
			this->radioButtonReal->TabStop = true;
			this->radioButtonReal->Text = L"Real";
			this->radioButtonReal->UseVisualStyleBackColor = true;
			this->radioButtonReal->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonBool
			// 
			this->radioButtonBool->AutoSize = true;
			this->radioButtonBool->Location = System::Drawing::Point(74, 78);
			this->radioButtonBool->Name = L"radioButtonBool";
			this->radioButtonBool->Size = System::Drawing::Size(46, 16);
			this->radioButtonBool->TabIndex = 3;
			this->radioButtonBool->TabStop = true;
			this->radioButtonBool->Text = L"Bool";
			this->radioButtonBool->UseVisualStyleBackColor = true;
			this->radioButtonBool->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonBin
			// 
			this->radioButtonBin->AutoSize = true;
			this->radioButtonBin->Location = System::Drawing::Point(74, 100);
			this->radioButtonBin->Name = L"radioButtonBin";
			this->radioButtonBin->Size = System::Drawing::Size(56, 16);
			this->radioButtonBin->TabIndex = 4;
			this->radioButtonBin->TabStop = true;
			this->radioButtonBin->Text = L"Binary";
			this->radioButtonBin->UseVisualStyleBackColor = true;
			this->radioButtonBin->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonSubset
			// 
			this->radioButtonSubset->AutoSize = true;
			this->radioButtonSubset->Location = System::Drawing::Point(74, 122);
			this->radioButtonSubset->Name = L"radioButtonSubset";
			this->radioButtonSubset->Size = System::Drawing::Size(58, 16);
			this->radioButtonSubset->TabIndex = 5;
			this->radioButtonSubset->TabStop = true;
			this->radioButtonSubset->Text = L"Subset";
			this->radioButtonSubset->UseVisualStyleBackColor = true;
			this->radioButtonSubset->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonMatrix
			// 
			this->radioButtonMatrix->AutoSize = true;
			this->radioButtonMatrix->Location = System::Drawing::Point(74, 144);
			this->radioButtonMatrix->Name = L"radioButtonMatrix";
			this->radioButtonMatrix->Size = System::Drawing::Size(55, 16);
			this->radioButtonMatrix->TabIndex = 6;
			this->radioButtonMatrix->TabStop = true;
			this->radioButtonMatrix->Text = L"Matrix";
			this->radioButtonMatrix->UseVisualStyleBackColor = true;
			this->radioButtonMatrix->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonEqn
			// 
			this->radioButtonEqn->AutoSize = true;
			this->radioButtonEqn->Location = System::Drawing::Point(74, 166);
			this->radioButtonEqn->Name = L"radioButtonEqn";
			this->radioButtonEqn->Size = System::Drawing::Size(67, 16);
			this->radioButtonEqn->TabIndex = 7;
			this->radioButtonEqn->TabStop = true;
			this->radioButtonEqn->Text = L"Equation";
			this->radioButtonEqn->UseVisualStyleBackColor = true;
			this->radioButtonEqn->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// buttonOK
			// 
			this->buttonOK->Location = System::Drawing::Point(405, 191);
			this->buttonOK->Name = L"buttonOK";
			this->buttonOK->Size = System::Drawing::Size(75, 23);
			this->buttonOK->TabIndex = 9;
			this->buttonOK->Text = L"OK";
			this->buttonOK->UseVisualStyleBackColor = true;
			this->buttonOK->Click += gcnew System::EventHandler(this, &FormInput::buttonOK_Click);
			// 
			// textBoxMain
			// 
			this->textBoxMain->Location = System::Drawing::Point(6, 18);
			this->textBoxMain->Name = L"textBoxMain";
			this->textBoxMain->Size = System::Drawing::Size(272, 19);
			this->textBoxMain->TabIndex = 8;
			// 
			// textBoxRow
			// 
			this->textBoxRow->Location = System::Drawing::Point(13, 18);
			this->textBoxRow->Name = L"textBoxRow";
			this->textBoxRow->Size = System::Drawing::Size(58, 19);
			this->textBoxRow->TabIndex = 9;
			// 
			// textBoxCol
			// 
			this->textBoxCol->Location = System::Drawing::Point(94, 18);
			this->textBoxCol->Name = L"textBoxCol";
			this->textBoxCol->Size = System::Drawing::Size(58, 19);
			this->textBoxCol->TabIndex = 10;
			// 
			// groupBoxBool
			// 
			this->groupBoxBool->Controls->Add(this->radioButtonFalse);
			this->groupBoxBool->Controls->Add(this->radioButtonTrue);
			this->groupBoxBool->Location = System::Drawing::Point(194, 165);
			this->groupBoxBool->Name = L"groupBoxBool";
			this->groupBoxBool->Size = System::Drawing::Size(167, 49);
			this->groupBoxBool->TabIndex = 10;
			this->groupBoxBool->TabStop = false;
			this->groupBoxBool->Text = L"Bool";
			// 
			// radioButtonFalse
			// 
			this->radioButtonFalse->AutoSize = true;
			this->radioButtonFalse->Location = System::Drawing::Point(94, 20);
			this->radioButtonFalse->Name = L"radioButtonFalse";
			this->radioButtonFalse->Size = System::Drawing::Size(48, 16);
			this->radioButtonFalse->TabIndex = 1;
			this->radioButtonFalse->TabStop = true;
			this->radioButtonFalse->Text = L"false";
			this->radioButtonFalse->UseVisualStyleBackColor = true;
			// 
			// radioButtonTrue
			// 
			this->radioButtonTrue->AutoSize = true;
			this->radioButtonTrue->Location = System::Drawing::Point(27, 20);
			this->radioButtonTrue->Name = L"radioButtonTrue";
			this->radioButtonTrue->Size = System::Drawing::Size(43, 16);
			this->radioButtonTrue->TabIndex = 0;
			this->radioButtonTrue->TabStop = true;
			this->radioButtonTrue->Text = L"true";
			this->radioButtonTrue->UseVisualStyleBackColor = true;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(75, 22);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(17, 12);
			this->label1->TabIndex = 11;
			this->label1->Text = L"×";
			// 
			// groupBoxMat
			// 
			this->groupBoxMat->Controls->Add(this->textBoxRow);
			this->groupBoxMat->Controls->Add(this->label1);
			this->groupBoxMat->Controls->Add(this->textBoxCol);
			this->groupBoxMat->Location = System::Drawing::Point(194, 99);
			this->groupBoxMat->Name = L"groupBoxMat";
			this->groupBoxMat->Size = System::Drawing::Size(167, 49);
			this->groupBoxMat->TabIndex = 12;
			this->groupBoxMat->TabStop = false;
			this->groupBoxMat->Text = L"Matrix size";
			// 
			// groupBoxGen
			// 
			this->groupBoxGen->Controls->Add(this->textBoxMain);
			this->groupBoxGen->Location = System::Drawing::Point(194, 34);
			this->groupBoxGen->Name = L"groupBoxGen";
			this->groupBoxGen->Size = System::Drawing::Size(286, 49);
			this->groupBoxGen->TabIndex = 13;
			this->groupBoxGen->TabStop = false;
			this->groupBoxGen->Text = L"Value";
			// 
			// radioButtonStr
			// 
			this->radioButtonStr->AutoSize = true;
			this->radioButtonStr->Location = System::Drawing::Point(74, 188);
			this->radioButtonStr->Name = L"radioButtonStr";
			this->radioButtonStr->Size = System::Drawing::Size(53, 16);
			this->radioButtonStr->TabIndex = 14;
			this->radioButtonStr->TabStop = true;
			this->radioButtonStr->Text = L"String";
			this->radioButtonStr->UseVisualStyleBackColor = true;
			this->radioButtonStr->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonInvalid
			// 
			this->radioButtonInvalid->AutoSize = true;
			this->radioButtonInvalid->Location = System::Drawing::Point(12, 12);
			this->radioButtonInvalid->Name = L"radioButtonInvalid";
			this->radioButtonInvalid->Size = System::Drawing::Size(56, 16);
			this->radioButtonInvalid->TabIndex = 15;
			this->radioButtonInvalid->TabStop = true;
			this->radioButtonInvalid->Text = L"Invalid";
			this->radioButtonInvalid->UseVisualStyleBackColor = true;
			this->radioButtonInvalid->CheckedChanged += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			// 
			// radioButtonEmpty
			// 
			this->radioButtonEmpty->AutoSize = true;
			this->radioButtonEmpty->Location = System::Drawing::Point(12, 34);
			this->radioButtonEmpty->Name = L"radioButtonEmpty";
			this->radioButtonEmpty->Size = System::Drawing::Size(55, 16);
			this->radioButtonEmpty->TabIndex = 16;
			this->radioButtonEmpty->TabStop = true;
			this->radioButtonEmpty->Text = L"Empty";
			this->radioButtonEmpty->UseVisualStyleBackColor = true;
			// 
			// FormInput
			// 
			this->AcceptButton = this->buttonOK;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(491, 222);
			this->Controls->Add(this->radioButtonEmpty);
			this->Controls->Add(this->radioButtonInvalid);
			this->Controls->Add(this->radioButtonStr);
			this->Controls->Add(this->groupBoxGen);
			this->Controls->Add(this->groupBoxMat);
			this->Controls->Add(this->groupBoxBool);
			this->Controls->Add(this->buttonOK);
			this->Controls->Add(this->radioButtonEqn);
			this->Controls->Add(this->radioButtonMatrix);
			this->Controls->Add(this->radioButtonSubset);
			this->Controls->Add(this->radioButtonBin);
			this->Controls->Add(this->radioButtonBool);
			this->Controls->Add(this->radioButtonReal);
			this->Controls->Add(this->radioButtonUnsigned);
			this->Controls->Add(this->radioButtonInt);
			this->Name = L"FormInput";
			this->Text = L"FormInput";
			this->Shown += gcnew System::EventHandler(this, &FormInput::OnTypeChanged);
			this->groupBoxBool->ResumeLayout(false);
			this->groupBoxBool->PerformLayout();
			this->groupBoxMat->ResumeLayout(false);
			this->groupBoxMat->PerformLayout();
			this->groupBoxGen->ResumeLayout(false);
			this->groupBoxGen->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void OnTypeChanged(System::Object^  sender, System::EventArgs^  e);
	private: System::Void buttonOK_Click(System::Object^  sender, System::EventArgs^  e);
};
}
