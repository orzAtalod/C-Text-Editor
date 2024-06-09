#include <string.h>

//using data structure SAM(suffix automaton) to search the strings along all files
//including the MOST stadard linked list in your life to satisfy those who is VERY nickpicking
////////////////////////////////////////// CONSTANTS /////////////////////////////////////////////

#define CHARNUM 30

////////////////////////////////////////// Structure Define //////////////////////////////////////

typedef struct baseSAM_Node {
	struct baseSAM_Node* link;
	struct baseSAM_Node* ch[CHARNUM];
	int len;
} SAM;

SAM* newSAM()
{
	return (SAM*)calloc(1, sizeof(SAM));
}

typedef struct baseTrie_Node {
	struct baseTrie_Node* ch[CHARNUM];
} Trie;

Trie* newTrie()
{
	return (Trie*)calloc(1, sizeof(Trie));
}

static SAM* fileSAM[65535];

/////////////////////////////////////////// Build SAM* From a Trie* //////////////////////////////

// A STANDARD LINKED LIST
typedef struct likedListBase {
	Trie* currentTrie;
	SAM*  currentSAM;
	linkedListBase* next;
} likedList;

struct {
	linkedList* head;
	linkedList* tail;
} BFSqueue;

static void BFSqueuePush(SAM* nSAM, Trie* nTrie)
{
	if(!BFSqueue.head)
	{
		BFSqueue.head = BFSqueue.tail = (linkedList*)malloc(sizeof(likedList));
	}
	else
	{
		BFSqueue.tail.next = (linkedList*)malloc(sizeof(likedList));
		BFSqueue.tail = BFSqueue.tail.next;
	}
	BFSqueue.tail->currentSAM  = nSAM;
	BFSqueue.tail->currentTrie = nTrie;
	BFSqueue.tail->next = 0;
}

static likedList* BFSqueuePop()
{
	if(!BFSqueue.head) return 0;
	
	linkedList* res = BFSqueue.head;
	if(BFSqueue.head == BFSqueue.tail)
	{
		BFSqueue.head = BFSqueue.tail = 0;
	}
	else
	{
		BFSqueue.head = BFSqueue.head.next;
	}
	
	return res;
}

static int queueIsEmpty()
{
	return BFSqueue.head != 0;
}

static void clearQueue()
{
	BFSqueue.head = BFSqueue.tail = 0;
}

static SAM* buildSAMFromTrie(Trie* theTrie)
{
	SAM* buildedSAM = newSAM();
	clearQueue();
	BFSqueuePush(buildedSAM, theTrie);

	while(!queueIsEmpty())
	{
		linkedList* now = BFSqueuePop();
		SAM* curSAM   = now->currentSAM;
		Trie* curTrie = now->currentTrie;

		for(int i=0; i<CHARNUM; ++i)
		{
			if(curTrie->ch[i])
			{
				SAM* nSAM = newSAM();
				nSAM->len = curSAM->len+1;

				SAM* p = curSAM;
				while(p && !p->ch[i])
				{
					p->ch[i] = nSAM;
					p = p->link;
				}

				if(p && p->ch[i])
				{
					if(p->ch[i]->len == p->len+1)
					{
						nSAM->link = p->ch[i];
					}
					else
					{
						SAM* q = p->ch[i];
						SAM* clone = newSAM();
						clone->len = p->len+1;
						clone->link = q->link;
						memcpy(clone->ch, q->ch, CHARNUM*sizeof(SAM*));
						while(p && p->ch[i]==q)
						{
							p->ch[i] = clone;
							p = p->link;
						}
						q->link = nSAM->link = clone;
					}
				}

				BFSqueuePush(nSAM, currentTrie->ch[i]);
			}
		}
	}

	return buildedSAM;
}

///////////////////////////////////////////////// build Trie* from Page ///////////////////////////

Trie* buildedTrie;

static void insertTrieFromBlock(Block* blk)
{
	if(blk->type != 1) return;
	StyleString* curss = (StyleString*)blk->dataptr;
	
	Trie* curTrie = buildedTrie;
	for(int i=0; i<curss->contentLen; ++i)
	{
		const char chr = curss->content[i]->content;
		if(!curTrie->ch[chr])
		{
			curTrie->ch[chr] = newTrie();
		}
		curTrie = curTrie->ch[chr];
	}
}

static Trie* buildTrieFromPage(int page)
{
	buildedTrie = newTrie();
	const int curblkPage = GetPageOfBlockList();
	ChangePageOfBlockList(page);
	TraverseBlockList(insertTrieFromBlock);
	ChangePageOfBlockList(curblkPage);
	return buildedTrie;
}

//////////////////////////////////////////////// build /////////////////////////////////////////////

static void searchBuildTraverse(FileHeaderInfo* fi, int page)
{
	Trie* t = buildTrieFromPage(page);
	fileSAM[fi->fileID] = buildSAMFromTrie(t);
}

void SearchBuild()
{
	BrowseExplorer(searchBuildTraverse, 0);
}

/////////////////////////////////////////////// search logic && KMP ///////////////////////////////

char searchText[255];
int  KMPnext[255];

static void buildKMP()
{
	if(!searchText[0] || !searchText[1]) return;
	for(int i=2; searchText[i]; ++i)
	{
		int p = KMPnext[i-1];
		while(p && searchText[p]!=searchText[i])
		{
			p = KMPnext[p];
		}
		KMPnext[i] = searchText[p]==searchText[i] ? p+1 : p;
	}
}

static int searchFromSAM(int fileID)
{
	SAM* theSAM = fileSAM[fileID];
	for(int i=0; txt[i]; ++i)
	{
		theSAM = theSAM->ch[txt[i]];
		if(!theSAM) return 0;
	}
	return 1;
}

static void searchBlockFromKMP(void* res, Block* blk)
{
	if(blk->type != 1) return;
	if(*((int*)res)) return;
	StyleString* ss = blk->dataptr;

	const int curp = 0;
	for(int i=0; i<ss->contentLen; ++i)
	{
		while(curp && searchText[curp]!=ss->content[i]->content)
		{
			curp = KMPnext[curp];
		}
		if(searchText[curp] == ss->content[i]->content)
		{
			curp = curp+1;
		}
		if(!searchText[curp])
		{
			*((int*)res) = 1;
			return;
		}
	}
}

static int searchFromKMP(int page)
{
	const int curPage = GetPageOfBlockList();
	ChangePageOfBlockList(page);

	int* res = (int*)malloc(sizeof(int));
	*res = 0;
	AccumlateBlockList(searchBlockFromKMP, res);

	const int ans = *res;
	free(res);
	ChangePageOfBlockList(curPage);
	return ans;
}

/////////////////////////////////////////////// search & draw /////////////////////////////////////

static int searchFilter(FileHeaderInfo* fi, int page)
{
	if(page)
	{
		return searchFromKMP(page);
	}
	else
	{
		return searchFromSAM(fi->fileID);
	}
}

DictionaryFolder* df;

void SearchString(const char* sTxt)
{
	if(sTxt) strcpy(searchText, sTxt);
	else searchText[0] = '\0';
	if(df) FreeDictionaryFolder(df);
	buildKMP();
	df = FilterExplorer(searchFilter, 1);
}

void SearchDraw(double cx, double cy, double dx, double dy)
{
	if(!df) return;
	DrawDictionaryList(0, df, cx, cy, dx-cx, 0, dy-cy);
}