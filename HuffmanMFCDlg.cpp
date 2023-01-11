
// HuffmanMFCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "HuffmanMFC.h"
#include "HuffmanMFCDlg.h"
#include "afxdialogex.h"
#include "Edit_text.h"
#include "JIEYA_FILENAME.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// 算法实现所用变量和数组
int bytes_cnt = 0, block_current = 0;
long long file_len = 0;
const int Fmax = 128, Smax = 10, Buffmax = 1024, Blockmax = 512;
char buff[Buffmax] = "";
char bufstr[Buffmax / 8] = "", block[Blockmax] = "";
char file_extension[MAX_PATH];//起始文件的扩展名
char dat_file_extension[MAX_PATH];/*二进制文件内的扩展名*/

// MFC搭载所需变量和数组
bool named_ok;//是否为生成文件命名
char cp_file_name[MAX_PATH];//起始文件的名字无扩展名
char dcp_file_name[MAX_PATH];//压缩文件的名字无扩展名
char cp_inname[MAX_PATH], cp_outname[MAX_PATH], dcp_inname[MAX_PATH], dcp_outname[MAX_PATH];

struct HaffNode {
	unsigned char byte; //节点代表的字符（ASCII码表对应的字符）
	long long w; // 每个节点代表字符的出现频度
	int num, fa, le, ri, code_len;
	// 节点在Huff_arr数组的下标，双亲节点在数组的下标，左孩子树下标，右孩子树下标，对应哈夫曼编码长度
	char code[256];
	bool operator < (HaffNode x) const { // 重载比较符号 <
		return x.w < w;
	}
}Huff_arr[520];

//利用缓冲区加速文件的读取
void flushBuffer(FILE* fp) { // 把缓冲区中，尽可能多的字节，写入文件中 
	strcpy(bufstr, "");
	unsigned char temp = 0;
	int byte_data_num = strlen(buff) / 8, i;
	for (i = 0; i < byte_data_num; i++) {
		temp = 0;
		for (int j = 0; j < 8; j++) {
			if (buff[i * 8 + j] == '1')
				temp += pow(2, 7 - j);
		}
		bufstr[i] = temp;
	}
	bufstr[i] = '\0';
	fwrite(bufstr, 1, byte_data_num, fp);
	strcpy(buff, buff + byte_data_num * 8);
}

// 构建哈夫曼树（最优树）
void createhufftree() {
	priority_queue<HaffNode> QUEUE;
	HaffNode First, Second, Sum;
	int tot = bytes_cnt; // 字符数组中最后一个字符的后一个
	// 清空队列
	while (QUEUE.size())
		QUEUE.pop();
	// 频度大于1的字符进入队列
	for (int i = 0; i < bytes_cnt; i++) {
		Huff_arr[i].num = i;
		QUEUE.push(Huff_arr[i]);
	}


	while (QUEUE.size() > 1) {
		// 获取权重最小的两个字符出队，必须保证first小于second
		First = QUEUE.top(), QUEUE.pop();
		Second = QUEUE.top(), QUEUE.pop();
		// 两个最小的结点生成一个新的根节点
		Sum.num = tot, Sum.w = First.w + Second.w;
		Sum.fa = -1, Sum.le = First.num, Sum.ri = Second.num;
		strcpy(Sum.code, "");  //设置哈夫曼编码为空
		// 将生成的根节点添加进Huff_arr数组中，并设置first和second的父母结点下标
		Huff_arr[First.num].fa = Sum.num, Huff_arr[Second.num].fa = Sum.num;
		Huff_arr[tot++] = Sum;
		QUEUE.push(Sum);  // 生成的父母结点入队
	}
}

// 构造哈夫曼编码
void createhuffcode() {
	int tot = bytes_cnt * 2 - 1;      //tot是根节点的后一个数组下标
	Huff_arr[tot - 1].code[0] = '\0'; //设置根节点的哈夫曼编码为空字符串
	for (int i = tot - 2; i >= 0; i--) {
		//把父母结点的哈夫曼编码复制到孩子的哈夫曼编码上
		strcpy(Huff_arr[i].code, Huff_arr[Huff_arr[i].fa].code);
		if (Huff_arr[Huff_arr[i].fa].ri == i)
			strcat(Huff_arr[i].code, "1"); //右孩子追加1
		else
			strcat(Huff_arr[i].code, "0"); //左孩子追加0
		Huff_arr[i].code_len = strlen(Huff_arr[i].code); //哈夫曼编码的长度
	}
}

//读取文件
void initpow(char* cp_inname) {
	unsigned char ch;
	CString str;

	// 读取文件，新建二进制文件存放统计数据
	FILE* ifp = fopen(cp_inname, "rb");
	if (ifp == NULL) {
		MessageBox(NULL, _T("文件已存在，请重新输入"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}

	// 读取源文件字节，统计次数w和文件长度
	file_len = 0, bytes_cnt = 0;
	fread(&ch, 1, 1, ifp);
	while (!feof(ifp)) {
		Huff_arr[ch].w++;
		file_len++;
		fread(&ch, 1, 1, ifp);
	}
	fclose(ifp);
	// 结点个数
	for (int i = 0; i < 256; i++) {
		if (Huff_arr[i].w > 0)
			bytes_cnt++;
	}
	// 排序，用于构建哈夫曼树
	sort(Huff_arr, Huff_arr + 511);
}

//压缩文件
void compressFile(char* cp_inname, char* cp_outname) {
	unsigned char ch;
	FILE* ifp, * ofp;
	//初始化哈夫曼树
	for (int i = 0; i < 511; i++) {
		Huff_arr[i].byte = i;
		Huff_arr[i].num = -1;
		Huff_arr[i].w = 0;
		Huff_arr[i].fa = Huff_arr[i].le = Huff_arr[i].ri = 0;
		strcpy(Huff_arr[i].code, "");
	}

	//读取文件并初始化权值记录
	initpow(cp_inname);

	//构造哈夫曼树
	createhufftree();

	//构造哈夫曼编码
	createhuffcode();

	// 生成压缩文件
	ofp = fopen(cp_outname, "wb");
	if (ofp == NULL) {
		MessageBox(NULL, _T("未能成功打开文件"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}
	fprintf(ofp, "%d,%s,%lld,%d,", strlen(file_extension), file_extension, file_len, bytes_cnt);
	for (int i = 0; i < bytes_cnt; i++)
		fprintf(ofp, "%c,%lld,", Huff_arr[i].byte, Huff_arr[i].w);

	ifp = fopen(cp_inname, "rb");
	if (ifp == NULL) {
		MessageBox(NULL, _T("打开文件失败"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}

	strcpy(buff, ""); //缓冲区置空
	ch = fgetc(ifp); //获取文件输入流的第一个字符
	while (!feof(ifp)) {
		if (Buffmax - strlen(buff) > 256) {
			for (int i = 0; i < bytes_cnt; i++) {
				if (Huff_arr[i].byte == ch) { //从频度数组中匹配文件的字符
					strcat(buff, Huff_arr[i].code); //缓冲区中追加该字符的哈夫曼编码
					ch = fgetc(ifp); //继续获取文件的下一个字符
					break;
				}
			}
		}
		else {
			flushBuffer(ofp);
		}
	}
	flushBuffer(ofp);
	if (strlen(buff) > 0) {
		strcat(buff, "00000000"); //缓冲区还有元素，补8个0
		flushBuffer(ofp);  //整除8，将多余的0去掉
		strcpy(buff, "");
	}
	fclose(ofp);
	fclose(ifp);
}

//解压文件
void deCompressFile(char* dcp_inname, char* dcp_outname) { //解压
	int sufname_len, tot, root, bycur = 0, bucur = 0, trcur, blcur = 0;
	long long dfile_len = 0;
	char ch;
	FILE* ifp, * ofp;
	//初始化哈夫曼树
	for (int i = 0; i < 511; i++)
		Huff_arr[i].byte = i, Huff_arr[i].num = -1, Huff_arr[i].w = 0, Huff_arr[i].fa = Huff_arr[i].le = Huff_arr[i].ri = -1, strcpy(Huff_arr[i].code, "");

	//读取压缩文件并还原哈夫曼树
	ifp = fopen(dcp_inname, "rb");
	if (ifp == NULL) {
		MessageBox(NULL, _T("此压缩文件不存在或被占用!"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}
	strcpy(dat_file_extension, "");
	fscanf(ifp, "%d,", &sufname_len);
	fread(&dat_file_extension, sufname_len, 1, ifp);
	fscanf(ifp, ",%lld,%d,", &file_len, &bytes_cnt);
	for (int i = 0; i < bytes_cnt; i++)
		fscanf(ifp, "%c,%lld,", &Huff_arr[i].byte, &Huff_arr[i].w);

	//构造哈弗曼树并输出
	createhufftree();

	//生成文件绝对路径
	strcat(dcp_outname, ".");
	strcat(dcp_outname, dat_file_extension);
	//解压
	ofp = fopen(dcp_outname, "wb");
	if (ofp == NULL) {
		MessageBox(NULL, _T("解压文件生成失败！"), _T("错误"), MB_ICONEXCLAMATION);
		return;
	}
	strcpy(buff, ""), strcpy(block, "");
	fread(buff, 1, Buffmax - 1, ifp);
	root = bytes_cnt * 2 - 2, trcur = root;
	while (dfile_len < file_len) {
		if (blcur >= Blockmax - 1) {
			fwrite(block, 1, blcur, ofp);
			blcur = 0;
		}
		if (Huff_arr[trcur].le == -1) {
			block[blcur++] = Huff_arr[trcur].byte, block[blcur] = '\0';
			trcur = root, dfile_len++;
			if (blcur == 510) {
				int xxdx = 1;
				xxdx++;
			}
		}
		else {
			if ((buff[bucur] >> (7 - bycur)) & 1)
				trcur = Huff_arr[trcur].ri;
			else
				trcur = Huff_arr[trcur].le;
			if (bycur < 7)
				bycur++;
			else {
				bycur = 0, bucur++;
				if (bucur >= Buffmax - 1)
					fread(buff, 1, Buffmax - 1, ifp), bucur = 0;
			}
		}
	}
	fwrite(block, 1, blcur, ofp);
	fclose(ifp), fclose(ofp);
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHuffmanMFCDlg 对话框



CHuffmanMFCDlg::CHuffmanMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HUFFMANMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1); // IDI_ICON1 修改窗口右上角图标
}

void CHuffmanMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHuffmanMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CHuffmanMFCDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CHuffmanMFCDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CHuffmanMFCDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CHuffmanMFCDlg 消息处理程序

BOOL CHuffmanMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CHuffmanMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHuffmanMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHuffmanMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHuffmanMFCDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFile = _T("");
	CString str3;
	str3.Format(_T("请选择所需要进行压缩的文件："));
	if (MessageBox(str3, _T("提示"), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
		return;
	}
	else {
		CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files All Files (*.*)|*.*||"), NULL);

		if (dlgFile.DoModal())
		{
			strFile = dlgFile.GetPathName();
		}
	}
	if (strFile == "")
		return;

	CString str4;
	Edit_text YS;
	str4.Format(_T("请选择是否为生成文件重新命名："));
	named_ok = false;
	//是否进行对生成文件的命名
	if (MessageBox(str4, _T("选择"), MB_ICONQUESTION | MB_YESNO) == IDNO) {
		named_ok = false;
	}
	else {
		named_ok = true;
		YS.DoModal();
	}

	//对文件名进行修改
	USES_CONVERSION;
	char* inFileName = T2A(strFile);
	int ok = 0;
	int pos1 = 0, pos2;
	strcpy(cp_file_name, "");
	strcpy(file_extension, "");
	char temp[MAX_PATH] = "";
	for (int i = strlen(inFileName) - 1; i >= 0; i--) {
		if (inFileName[i] == '\\') {
			break;
		}
		if (ok == 1) {
			temp[pos1++] = inFileName[i];
		}
		if (inFileName[i] == '.') {
			pos2 = i + 1;
			ok = 1;
		}
	}
	int tot = 0;
	for (int i = pos2; i < strlen(inFileName); i++)
		file_extension[tot++] = inFileName[i];
	int num = 0;
	for (int i = pos1 - 1; i >= 0; i--) {
		cp_file_name[num++] = temp[i];
	}
	strcat(cp_file_name, ".dat");
	CString NAME = YS.FILE_NAME + ".dat";//文本框传来的信息
	if (!named_ok)
		NAME = CA2CT(cp_file_name);
	char szPath[MAX_PATH];//存放选择的目录路径
	CString str1, str2, FileName;
	CTime m_time;
	ZeroMemory(szPath, sizeof(szPath));
	BROWSEINFO bi;
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = (LPWSTR)szPath;
	bi.lpszTitle = _T("请选择生成文件的目录：");
	bi.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_NEWDIALOGSTYLE;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	FileName = m_time.Format(NAME);
	SHGetPathFromIDList(lp, (LPWSTR)szPath);
	str2.Format(_T("%s"), szPath);
	CString filePath = str2 + "\\" + FileName;//路径+文件名
	if (lp && SHGetPathFromIDList(lp, (LPWSTR)szPath))
	{
		str1.Format(_T("选择生成文件的路径为： %s"), szPath);
		if (MessageBox(str1, _T("路径"), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
			return;
		}
		else {
			USES_CONVERSION;
			//函数T2A和W2A均支持ATL和MFC中的字符
			char* outFileName = T2A(filePath);
			clock_t  clockBegin, clockEnd;
			clockBegin = clock();
			CFile cfile;
			DOUBLE size1, size2;
			if (cfile.Open(strFile, CFile::modeRead))
			{
				size1 = cfile.GetLength();
			}
			cfile.Close();
			compressFile(inFileName, outFileName);
			clockEnd = clock();
			DOUBLE TIME = (clockEnd - clockBegin) / (CLOCKS_PER_SEC);
			if (cfile.Open(filePath, CFile::modeRead))
			{
				size2 = cfile.GetLength();
			}
			cfile.Close();
			UpdateData(FALSE);
			CString TIMESTR;
			size1 /= 1024;
			size2 /= 1024;
			DOUBLE YSL = size2 / size1 * 100;
			char s = '%';
			TIMESTR.Format(_T("压缩文件耗时为：%.2lfs\n起始文件大小为：%.2lfKB\n压缩文件大小为：%.2lfKB\n文件的压缩率为：%.2lf%c"), TIME, size1, size2, YSL, s);
			MessageBox(TIMESTR, _T("压缩成功"));
		}
	}
	else
	{
		AfxMessageBox(_T("无效的目录，请重新选择"));
		return;
	}
}


void CHuffmanMFCDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFile = _T("");
	CString str3;
	str3.Format(_T("请选择所需要进行解压的文件："));
	if (MessageBox(str3, _T("提示"), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
		return;
	}
	else {
		CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files All Files (*.*)|*.*||"), NULL);

		if (dlgFile.DoModal())
		{
			strFile = dlgFile.GetPathName();
		}
	}
	if (strFile == "")
		return;
	CString str4;
	JIEYA_FILENAME JY;
	str4.Format(_T("请选择是否为生成文件重新命名："));
	named_ok = false;
	if (MessageBox(str4, _T("选择"), MB_ICONQUESTION | MB_YESNO) == IDNO) {
		named_ok = false;
	}
	else {
		named_ok = true;
		JY.DoModal();
	}
	//对文件名进行修改
	USES_CONVERSION;
	char* inFileName = T2A(strFile);
	int ok = 0;
	int pos = 0;
	strcpy(dcp_file_name, "");
	char temp[MAX_PATH] = "";
	for (int i = strlen(inFileName) - 1; i >= 0; i--) {
		if (inFileName[i] == '\\') {
			break;
		}
		if (ok == 1) {
			temp[pos++] = inFileName[i];
		}
		if (inFileName[i] == '.') {
			ok = 1;
		}
	}
	int num = 0;
	for (int i = pos - 1; i >= 0; i--) {
		dcp_file_name[num++] = temp[i];
	}
	CString NAME = JY.TEXT_NAME;//文本框传来的信息
	if (!named_ok)
		NAME = CA2CT(dcp_file_name);
	char szPath[MAX_PATH];//存放选择的目录路径
	CString str1, str2, FileName;
	CTime m_time;
	ZeroMemory(szPath, sizeof(szPath));
	BROWSEINFO bi;
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = (LPWSTR)szPath;
	bi.lpszTitle = _T("请选择生成文件的目录：");
	bi.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_NEWDIALOGSTYLE;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	FileName = m_time.Format(NAME);
	SHGetPathFromIDList(lp, (LPWSTR)szPath);
	str2.Format(_T("%s"), szPath);
	CString filePath = str2 + "\\" + FileName;//路径+文件名无扩展名
	if (lp && SHGetPathFromIDList(lp, (LPWSTR)szPath))
	{
		str1.Format(_T("选择生成文件的路径为： %s"), szPath);
		if (MessageBox(str1, _T("路径"), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
			return;
		}
		else {
			USES_CONVERSION;
			char* outFileName = T2A(filePath);
			clock_t  clock1, clock2;
			clock1 = clock();
			deCompressFile(inFileName, outFileName);
			clock2 = clock();
			DOUBLE TIME = (clock2 - clock1) / (CLOCKS_PER_SEC);
			CString TIMESTR;
			UpdateData(FALSE);
			TIMESTR.Format(_T("\t解压成功！！！\t\n\t解压文件耗时为：%.2lfs\t"), TIME);
			MessageBox(TIMESTR, _T("信息提示"));
		}
	}
	else
	{
		AfxMessageBox(_T("无效的目录，请重新选择"));
		return;
	}
}


void CHuffmanMFCDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str3;
	str3.Format(_T("是否确定要退出程序？")); //为新弹出的对话框设置对话
	if (MessageBox(str3, _T("提醒"), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
		return;
	}
	else {
		PostQuitMessage(0);
	}
}
