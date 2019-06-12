#include "stdafx.h"
#include "UpdateManager.h"
#include "FormUnivTypeList.h"
#include "FormMatrixView.h"
#include "CdbException.h"

using namespace ConfigBrowser;

UpdateManager::UpdateManager()
{
#if 0	// static�ȃN���X�̃R���X�g���N�^���N�����Ɏ����I�ɌĂяo��������@��������Ȃ�
	m_pUpdateListenerList = gcnew array<System::Windows::Forms::Form^>(256);
	m_count = 0;
	for each(System::Windows::Forms::Form^ form in m_pUpdateListenerList){
		form = nullptr;
	}
#endif
}

void UpdateManager::AddToUpdateList(int type, System::Windows::Forms::Form^ pObj)
{
	// �ŏ���1��̂Ƃ��Ƀ��X�g�𐶐�
	if(m_pUpdateListenerList == nullptr){
		m_pUpdateListenerList = gcnew array<System::Windows::Forms::Form^>(256);
		m_count = 0;
		for each(System::Windows::Forms::Form^ form in m_pUpdateListenerList){
			form = nullptr;
		}
	}

	pObj->Tag = type;
	m_pUpdateListenerList[m_count++] = pObj;
}

void UpdateManager::RemoveFromUpdateList(System::Windows::Forms::Form^ pObj)
{
	int i;
	for(i = 0; i < m_count; i++){
		if(m_pUpdateListenerList[i] == pObj) break;
	}
	for(; i < m_pUpdateListenerList->Length - 1; i++){
		m_pUpdateListenerList[i] = m_pUpdateListenerList[i + 1];
	}
}

void UpdateManager::NotifyUpdate()
{
	try{
		for each(System::Windows::Forms::Form^ form in m_pUpdateListenerList){
			if(form == nullptr) continue;
			switch((int)form->Tag){
				case WT_FORM_UNIVTYPE_LIST:
					dynamic_cast<FormUnivTypeList^>(form)->UpdateUnivTypeList();
					break;
				case WT_FORM_MATRIX_VIEW:
					dynamic_cast<FormMatrixView^>(form)->UpdateUnivTypeList();
					break;
				default:
					break;
			}
		}
	}
	catch(CdbException& e){
		MessageBox::Show(gcnew System::String(e.what()), "ERROR"); 
	}
}
