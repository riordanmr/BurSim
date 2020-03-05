// Screen.h - Header file for BurSim screen database class

#define BSSCREEN_HEADER_LEN  3

// These are ASCII versions of T27 special characters.
#define CHAR_BEG_UNPROT_FIELD       '\x1f'
#define CHAR_END_UNPROT_FIELD       '\x1e'
#define CHAR_BEG_TRANS_PROMPT       '\x1c'
#define CHAR_BRIGHT_TO_EOL          '\x1a'

#define T27HEADER    "\x1bX\x0c"
#define T27TRAILER   "\x1bW\x03"

enum enum_field_type {FLD_OUTPUT_ONLY, FLD_UNPROT, FLD_TRANS_PROMPT};

class CBSField {
public:
   CString  m_strFieldName;
   int      m_field_start;
   int      m_field_len;
   int      m_field_input_offset;// Number of bytes from beg of response from 
                                 // user for the first char of this field.
                                 // This is equal to the sum of the sizes of all
                                 // prior unprotected and transmittable prompt 
                                 // fields on this form.
   enum enum_field_type m_field_type;
   BOOL     m_bAlias;
public:
   CBSField();
};

class CBSForm {
public:
   CArray<CBSField, CBSField> m_aryFields;
   CString  m_strFormName;
   CString  m_strFormContents;
   int      m_nInputCharsSoFar;  // While the form definition is being cracked,
                                 // 
public:
   CBSForm();
   void operator = (const CBSForm &other);
   CBSForm(const CBSForm &other);  // Copy constructor
   int  CrackFieldOnForm(CString &restofline, CString &strName, char &endch);
   void CrackFormLine(CString formline);
};

class CBSFormFile {
public:
   CArray<CBSForm, CBSForm &> m_aryForms;
   CString     m_strFilename;
public:
   CBSFormFile();
   int ReadScreenFile(const char *szFilename);
   void CBSFormFileCopy(const CBSFormFile &other, CBSFormFile &newone);
   CBSFormFile operator = (const CBSFormFile &other);
   CBSFormFile(const CBSFormFile &other);  // Copy constructor
   void Dump();
};