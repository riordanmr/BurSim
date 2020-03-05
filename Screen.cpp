// Screen.cpp
//
// Implement the BurSim screen (forms) database class.
//
// Mark Riordan  6 Sep 1999
//
// A forms input file contains definitions for one or more forms.
// This standard ASCII file has one or more sections, each corresponding 
// to one form.
// For a each form, the syntax is:
//
//   !Form formname
//     (textual representation of form--up to 24 lines)
//   !EndForm
//
// Within the textual representation, the following characters have
// special meaning:
//   {  starts an output-only field.
//      The characters following the { are the name of the field.
//   }  ends an output-only field.
//   [  starts a left-justified input field.  This is a field to 
//      which the user can tab.  On a T27, this would be a US.
//   `  starts a transmittable prompt (i.e., protected field).
//      This text is sent when the user hits the Transmit key, but the
//      user can't change it.  On a T27, this is a FS.
//   ]  ends a field started by [ or `.  On a T27, this is an RS.

#include "stdafx.h"

extern int Debug;

//-----------------------------------------------------------------------
inline UINT AFXAPI HashKey(CString key)
{
	UINT nHash = 0;
      int nchars = key.GetLength();
	while (nchars--)
		nHash = (nHash<<5) + nHash + key.GetAt(nchars);
	return nHash;
}
//-----------------------------------------------------------------------

CBSField::CBSField()
{
   m_bAlias = FALSE;
}

CBSForm::CBSForm() {
   m_nInputCharsSoFar = 0;
}


// Copy constructor
CBSForm::CBSForm(const CBSForm &other)
{
   m_strFormName = other.m_strFormName;
   m_strFormContents = other.m_strFormContents;
   int size = other.m_aryFields.GetSize();
   for(int j=0; j<size; j++) {
      m_aryFields.Add(other.m_aryFields[j]);
   }
}

void CBSForm::operator =(const CBSForm &other)
{
   m_strFormName = other.m_strFormName;
   m_strFormContents = other.m_strFormContents;
   int size = other.m_aryFields.GetSize();
   for(int j=0; j<size; j++) {
      m_aryFields.Add(other.m_aryFields[j]);
   }
}

/*--- function CBSForm::CrackFieldOnForm -----------------------------------
 *
 *  Examines a line on a form and cracks off the first field, if any.
 *  Entry:  restofline  is the remainder of the line on the form, starting
 *                      after the character (such as { or [) that started the field.
 *          
 *  Exit:   restofline  is the rest of the line, after cracking off the
 *                      entire field, including the starting char, the
 *                      contents of the field, and the ending char if any.
 *          Returns the length of the field, not including delimiters.
 */
int CBSForm::CrackFieldOnForm(CString &restofline, CString &strName, char &endch)
{
   int fieldlen;

   strName.Empty();
   CString strContents = restofline.SpanExcluding("[]{}~");
   fieldlen = strContents.GetLength();
   restofline = restofline.Mid(fieldlen);
   if(!fieldlen) {
      return 0;
   }
   endch = 0;
   if(restofline.GetLength()) {
      endch = restofline.GetAt(0);
      if(strchr("]}~",endch)) {
         restofline = restofline.Mid(1);
      }
   }
   strName = strContents;
   strName.TrimRight();
   return fieldlen;
}

/*--- function CBSForm::CrackFormLine -----------------------------------
 *  Crack a single line on a form.
 *
 *  Entry:  formline    is the line.
 *
 *  Exit:   m_strFormContents has been updated.
 *
 *    [  starts an unprotected field.
 *    ]  ends   an unprotected field.
 *    {  starts an output field.
 *    }  ends   an output field.
 *    `  starts a  transmittable prompt.
 *    ~  maps to a "start bright" character.
 */
void CBSForm::CrackFormLine(CString formline)
{
   char fieldch, endch;
   int len;
   do {
      CString part=formline.SpanExcluding("[]{}`~");
      m_strFormContents += part;
      formline = formline.Mid(part.GetLength());
      len = formline.GetLength();
      fieldch = 0;
      if(len) {
         fieldch = formline.GetAt(0);
         formline = formline.Mid(1);
         len--;
      }
      switch(fieldch) {
      case '[':
        {
         CBSField field;
         field.m_field_type = FLD_UNPROT;
         m_strFormContents += CHAR_BEG_UNPROT_FIELD;
         field.m_field_len = CrackFieldOnForm(formline, field.m_strFieldName,endch);
         field.m_field_input_offset = m_nInputCharsSoFar;
         m_nInputCharsSoFar += field.m_field_len;
         field.m_field_start = m_strFormContents.GetLength();
         m_strFormContents += CString(' ',field.m_field_len);
         if(']'==endch) {
            m_strFormContents += CHAR_END_UNPROT_FIELD;
         }
         m_aryFields.Add(field);
        }
         break;
      case '`':
        {
         CBSField field;
         field.m_field_type = FLD_TRANS_PROMPT;
         m_strFormContents += CHAR_BEG_TRANS_PROMPT;
         field.m_field_len = CrackFieldOnForm(formline, field.m_strFieldName,endch);
         field.m_field_input_offset = m_nInputCharsSoFar;
         m_nInputCharsSoFar += field.m_field_len;
         field.m_field_start = m_strFormContents.GetLength();
         m_strFormContents += CString(' ',field.m_field_len);
         if(']'==endch) {
            m_strFormContents += CHAR_END_UNPROT_FIELD;
         }
         m_aryFields.Add(field);
        }
         break;
      case '{':
        {
         CBSField field;
         field.m_field_type = FLD_OUTPUT_ONLY;
         field.m_field_len = 1+CrackFieldOnForm(formline, field.m_strFieldName,endch);
         field.m_field_start = m_strFormContents.GetLength();
         m_strFormContents += CString(' ',field.m_field_len);
         if(endch) {
            // Account for ending }
            field.m_field_len++;
            m_strFormContents += " ";
         }
         m_aryFields.Add(field);
        }
         break;
      case '~':
         m_strFormContents += CHAR_BRIGHT_TO_EOL;
         break;
      }
   } while (formline.GetLength());
}

//===  CBSFormFile  ===================================================

CBSFormFile::CBSFormFile()
{
   m_strFilename.Empty();
   m_aryForms.RemoveAll();
}

void CBSFormFile::CBSFormFileCopy(const CBSFormFile &other, CBSFormFile &newone)
{
   newone.m_strFilename = other.m_strFilename;
   int size = other.m_aryForms.GetSize();
   for(int j=0; j<size; j++) {
      newone.m_aryForms.Add(other.m_aryForms[j]);
   }
}

CBSFormFile CBSFormFile::operator = (const CBSFormFile &other)
{
   CBSFormFileCopy(other,*this);
   return *this;
}

// Copy constructor
CBSFormFile::CBSFormFile(const CBSFormFile &other)
{
   CBSFormFileCopy(other,*this);
}

int CBSFormFile::ReadScreenFile(const char *szFilename)
{
   enum enum_read_state {ST_BEG, ST_IN_FORM} state = ST_BEG;
   char line[256];
   CString strline, strScreenName, strAlias, strReferredTo;
   CBSForm bsform;
   int iform=-1, retval=0, len, ifld;

   m_strFilename = szFilename;
   FILE *stin = fopen(szFilename,"r");
   if(!stin) return 1;
   while(fgets(line, sizeof line, stin)) {
      if(';' == line[0]) continue;
      strline = line;
      // Chop trailing newline.
      strline = strline.Left(strline.GetLength()-1);
      strline.TrimRight();
      switch (state) {
      case ST_BEG:
         if(matchbegi("!Form ",line)) {
            strScreenName = strline.Mid(6);
            strScreenName.TrimLeft();
            bsform.m_strFormName = strScreenName;
            m_aryForms.Add(bsform);
            iform = m_aryForms.GetSize()-1;
            state = ST_IN_FORM;
         } else if(matchbegi("!Alias ",line)) {
            strAlias = strline.Mid(7).SpanExcluding("=");
            strReferredTo = strline.Mid(7+strAlias.GetLength()+1);
            strAlias.TrimLeft();
            strAlias.TrimRight();
            strReferredTo.TrimLeft();
            strReferredTo.TrimRight();
            CBSForm &Form = m_aryForms[iform];
            BOOL bFound=FALSE;
            // Scan the array of fields in this database, looking for this field.
            for(ifld=0; !bFound && ifld<Form.m_aryFields.GetSize(); ifld++) {
               CBSField &Field = Form.m_aryFields[ifld];
               if(0==Field.m_strFieldName.CompareNoCase(strReferredTo)) {
                  // This is the field referred to.
                  // Make a copy of it and give it the alias name.
                  bFound = TRUE;
                  CBSField newField;
                  newField = Field;
                  newField.m_strFieldName = strAlias;
                  newField.m_bAlias = TRUE;
                  Form.m_aryFields.Add(newField);
                  if(Debug>2) FmtDebug("  Form '%s': added alias %s", Form.m_strFormName, strAlias);
               }
            }
            if(!bFound && Debug) FmtDebug(" ** Error: Form '%s': Could not find alias '%s'", Form.m_strFormName, strAlias);
         }
         break;
      case ST_IN_FORM:
         if(matchbegi("!EndForm", line)) {
            state = ST_BEG;
         } else {
            len = strline.GetLength();
            int padlen = MAXLINELEN-len > 0 ? MAXLINELEN-len : 0;
            CString formline = strline.Left(len) + CString(' ',padlen);
            m_aryForms[iform].CrackFormLine(formline);
         }
         break;
      }
   }
   fclose(stin);
   return retval;
}

void CBSFormFile::Dump()
{
   FmtDebug("Dumping form file '%s':",m_strFilename);
   int iform, ifld, nForms = m_aryForms.GetSize(), j;
   for(iform=0; iform<nForms; iform++) {
      FmtDebug("  Dumping form '%s'.  Contents=",m_aryForms[iform].m_strFormName);
      CString strContents = m_aryForms[iform].m_strFormContents;
      for(j=0; j<strContents.GetLength(); j += MAXLINELEN) {
         int nchars = MAXLINELEN;
         if(strContents.GetLength() - j < MAXLINELEN) {
            nchars = strContents.GetLength() - j;
         }
         FmtDebug("    %s",strContents.Mid(j,nchars));
      }
      int nFields = m_aryForms[iform].m_aryFields.GetSize();
      FmtDebug("  '%s' has %d fields:",m_aryForms[iform].m_strFormName, nFields);
      for(ifld=0; ifld<nFields; ifld++) {
         CBSField &Field = m_aryForms[iform].m_aryFields[ifld];
         FmtDebug("    '%s' start %4d len %2d type %d",Field.m_strFieldName,
            Field.m_field_start,Field.m_field_len,Field.m_field_type);
      }
   }
}