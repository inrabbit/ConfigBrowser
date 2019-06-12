#ifndef DEF_EMBEDDED_FUNC
#define DEF_EMBEDDED_FUNC

class UnivType;

typedef void (*CustomFunc)(UnivType& result, const UnivType **pArgs, int nArg);

struct EmbeddedFuncMap{
	int m_id;					// 組み込み関数ID
	const char *m_pName;		// 組み込み関数名称
	CustomFunc m_pfnFunc;		// 組み込む関数定義
	unsigned int m_DependArgsValue;	// 結果が依存する引数の指定(値のみの変更)
	unsigned int m_DependArgsSize;	// 結果が依存する引数の指定(行列のサイズ変更)
};

extern struct EmbeddedFuncMap g_EmbeddedFuncMap[];

#endif
