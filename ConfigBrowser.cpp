// ConfigBrowser.cpp : メイン プロジェクト ファイルです。

#include "stdafx.h"
#include "FormUnivTypeList.h"
#include "CdbException.h"

using namespace ConfigBrowser;

#define FILE_NAME "script.txt"

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// コントロールが作成される前に、Windows XP ビジュアル効果を有効にします
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// ファイル名を決定する
	System::String^ FileName;
	if(args->GetLength(0) >= 2){
		FileName = args[1]->ToString();
	}else{
		// デフォルトで開くファイル
		//FileName = gcnew System::String(FILE_NAME);
	}

#ifdef _DEBUG	// デバッグ用に標準出力をファイル出力に設定
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

		// メイン ウィンドウを作成して、実行します
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
