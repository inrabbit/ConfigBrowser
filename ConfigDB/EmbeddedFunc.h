#ifndef DEF_EMBEDDED_FUNC
#define DEF_EMBEDDED_FUNC

class UnivType;

typedef void (*CustomFunc)(UnivType& result, const UnivType **pArgs, int nArg);

struct EmbeddedFuncMap{
	int m_id;					// �g�ݍ��݊֐�ID
	const char *m_pName;		// �g�ݍ��݊֐�����
	CustomFunc m_pfnFunc;		// �g�ݍ��ފ֐���`
	unsigned int m_DependArgsValue;	// ���ʂ��ˑ���������̎w��(�l�݂̂̕ύX)
	unsigned int m_DependArgsSize;	// ���ʂ��ˑ���������̎w��(�s��̃T�C�Y�ύX)
};

extern struct EmbeddedFuncMap g_EmbeddedFuncMap[];

#endif
