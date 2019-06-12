#ifndef DEF_COMMON_UTILS
#define DEF_COMMON_UTILS

// 共通ユーティリティ

// NULL定義
#ifndef NULL
#define NULL 0
#endif

// TRUE, FALSEの定義
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// キー登録の最大数
#define CDB_MAX_KEY 500
// 配列要素の最大数
#define CDB_ARRAY_MAX 512
// 読み込みファイルサイズの最大
#define CDB_MAX_FILE_SIZE (180*1024)
// １つのトークンの最大サイズ
#define CDB_MAX_TOKEN_SIZE (80*3)
// 1行の最大サイズ
#define CDB_LINE_MAX_CHAR 512
// 登録可能なバイナリオブジェクトの最大数
#define CDB_MAX_BINARY_SIZE (200*1024)

#define CDB_TOKEN_EMPTY 1

#endif
