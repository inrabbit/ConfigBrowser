// ConfigBrowser.cpp : ���C�� �v���W�F�N�g �t�@�C���ł��B

#include "stdafx.h"
#include "FormUnivTypeList.h"
#include "CdbException.h"

using namespace ConfigBrowser;

#define FILE_NAME "script.txt"

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// �R���g���[�����쐬�����O�ɁAWindows XP �r�W���A�����ʂ�L���ɂ��܂�
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// �t�@�C���������肷��
	System::String^ FileName;
	if(args->GetLength(0) >= 2){
		FileName = args[1]->ToString();
	}else{
		// �f�t�H���g�ŊJ���t�@�C��
		//FileName = gcnew System::String(FILE_NAME);
	}

#ifdef _DEBUG	// �f�o�b�O�p�ɕW���o�͂��t�@�C���o�͂ɐݒ�
	System::IO::StreamWriter^ sw = gcnew System::IO::StreamWriter("cdb_debug.txt");
	sw->AutoFlush = true;
	System::IO::TextWriter^ tw = System::IO::TextWriter::Synchronized(sw);
	System::Console::SetOut(tw);
#endif

	FormUnivTypeList^ form = gcnew FormUnivTypeList;

	try{
		if(!FileName->Empty){
			form->SetFileName(FileName);
		}else{
			form->Text = "DSDL Browser";
		}

		// ���C�� �E�B���h�E���쐬���āA���s���܂�
		Application::Run(form);
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

	return 0;
} 
