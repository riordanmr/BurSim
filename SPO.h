// SPO.h 

// Number of screen rows left over after system info at top.
#define ROWS_DISPLAYED     19
// In SPO display, number of SPO messages that can be shown.
// ("-----[  System Messages  ]-----" takes up one line)
#define SPO_MSGS_DISPLAYED 18

#define MAX_SPO_MSGS       1000

// Zero-based Index of first row on screen on which info is displayed..
#define SPO_IDX_FIRST      5

struct struct_spo_msg {
   char spo_flags;
   char msg[MAXLINELEN+4];
};

class CSPO {
public:
   struct_spo_msg m_spo_msgs[MAX_SPO_MSGS];
   int            m_spo_next_idx;
   int            m_n_wrapped;  // # of times we have wrapped around MAX_SPO_MSGS

   CRITICAL_SECTION m_critSPO;

   CSPO();
   void LogSPOMsg(const char *flags, const char *text);
};

