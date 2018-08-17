#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<regex>
#include<iostream>
#include<unordered_map>
#include<algorithm>
#include<fstream>
#include<sstream>
#include <chrono>
using namespace std;

enum class TermType{
			VARIABLE,CONSTANT
		};
typedef unordered_map<string,string> Theta;

int max(int a, int b)
{
	return a>b?a:b;
}
class Term
{
	public:
		string name;
		
		TermType type;
		Term(string name)
		{	
			this->name = name;
			if(name[0]>=65 && name[0]<=90)
				this->type = TermType::CONSTANT;
			else
				this->type = TermType::VARIABLE;
		}
		Term* copy()
		{
			return new Term(name);
		}
		
};
class Atom  
{
	public:
		string predicate;
		vector<Term*> argList;
		bool isNeg;
		Atom(string predicate,vector<Term*> argList,bool isNeg)
		{	
			this->predicate = predicate;
			this->argList = argList;
			this->isNeg = isNeg;
		}
		Atom* copy()
		{
			vector<Term*> copyList;
			for(int i = 0;i<argList.size();i++)
			{
				copyList.push_back(argList[i]->copy());
			}
			return new Atom(predicate,copyList,isNeg);
		}
		
};
struct AtomComparer
{
	
	bool ValToBool(int val)
	{
		if(val<0)
			return true;
		return false;
	}
	bool operator()(const Atom *a,const Atom *b)
	{

			int val = (a->predicate).compare(b->predicate);
			
			if(val!=0)
			{
				//cout<<"Crazy"<<endl;
				return ValToBool(val);
				//cout<<"Crazier"<<endl;
			}
			
			int argSize = (a->argList).size();
			for(int i = 0 ;i<argSize;i++)
			{
				val = TermCompare(a->argList[i],b->argList[i]);
				if(val!=0)
					return ValToBool(val);
			}
			if(a->isNeg!=b->isNeg && a->isNeg)
				val = -1;
			else if(a->isNeg!=b->isNeg && b->isNeg)
				val = 1;
			if(val!=0)
					return ValToBool(val);
		
			return ValToBool((a->predicate).compare(b->predicate));
	}
	int TermCompare(const Term *a,const Term *b)
	{
		if(a->type == TermType::VARIABLE && b->type == TermType::VARIABLE)
		{
			return 0;
		}else
		if(a->type == TermType::VARIABLE && b->type == TermType::CONSTANT)
		{
			return 1;
		}else
		if(b->type == TermType::VARIABLE && a->type == TermType::CONSTANT)
		{
			return -1;
		}else
		{
			return (a->name).compare(b->name);
		}
		//if(b.type == TermType::CONSTANT && a.type == TermType::CONSTANT)
	}
}atomComparer;

class Clause
{
		
	public:
		vector<Atom*> atoms;
		bool isQuery;
		Clause(vector<Atom*> atoms)
		{
			sort(atoms.begin(),atoms.end(),atomComparer);
			this->atoms= atoms;
			//sorting atoms
			
		}
		int getAtom(string predicate,bool isNeg)
		{
			int n =  atoms.size();
			for(int i = 0;i<n ;i++)
			{
				if((atoms[i]->predicate)==predicate && (atoms[i]->isNeg)==isNeg )
					return i;
			}
			return -1;
		}
		int getAtomBinary(string predicate,bool isNeg)
		{
			int low = 0;
			int high = atoms.size()-1;
			while(low<=high)
			{
				int mid = low+(high-low)/2;
				int val = (atoms[mid]->predicate).compare(predicate);
				if(val == 0 && (atoms[mid]->isNeg)==isNeg )
					return mid;
				else if(val>0 || (val == 0 && isNeg==true))
				{
					high = mid-1;
				}
				else
				{
					low=mid+1;
				}
			}
			return -1;
		}
		
		Clause* copy()
		{
			vector<Atom*> copyList;
			
			for(int i = 0;i<atoms.size();i++)
			{
				copyList.push_back(atoms[i]->copy());
			}
			Clause *newClause = new Clause(copyList);
			newClause->isQuery = this->isQuery;
			return newClause;
		}
};
void printClause(const Clause *clause)
{
	
	if((clause->atoms).size()==0)
		cout<<"Goal Clause"<<endl;
	for(int i = 0;i<(clause->atoms).size();i++)
	{
		if(clause->atoms[i]->isNeg)	printf("~");

		cout<<clause->atoms[i]->predicate<<"(";
		for(int j = 0;j<(clause->atoms[i]->argList).size();j++)
		{
			cout<<clause->atoms[i]->argList[j]->name<<",";
		}
		cout<<")"<<" | ";
	}
	cout<<endl;
}
void deleteClause(const Clause *clause)
{
	
	for(int i = 0;i<(clause->atoms).size();i++)
	{
		//if(clause->atoms[i]->isNeg)	printf("~");

		//cout<<clause->atoms[i]->predicate<<"(";
		for(int j = 0;j<(clause->atoms[i]->argList).size();j++)
		{
			delete(clause->atoms[i]->argList[j]);
		}
		delete(clause->atoms[i]);
	}
	delete(clause);
//	cout<<endl;
}
struct ClauseComparer
{
	void swap(Clause** a, Clause** b)
	{
    	Clause* t = *a;
    	*a = *b;
    	*b = t;
	}
	int partition (vector<Clause*>& arr, int low, int high)
	{
    	Clause* pivot = arr[high];    // pivot
    	int i = (low - 1);  // Index of smaller element
 
    	for (int j = low; j <= high- 1; j++)
    	{
        	// If current element is smaller than or
        	// equal to pivot
        	if (comp(arr[j],pivot))
        	{
            	i++;    // increment index of smaller element
            	swap(&arr[i], &arr[j]);
        	}
    	}
    	swap(&arr[i + 1], &arr[high]);
    	return (i + 1);
	}
 
/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
	void quickSort(vector<Clause*>& arr, int low, int high)
	{
    	if (low < high)
    	{
        	/* pi is partitioning index, arr[p] is now
         	  at right place */
        	int pi = partition(arr, low, high);
 
        	// Separately sort elements before
        	// partition and after partition
        	quickSort(arr, low, pi - 1);
        	quickSort(arr, pi + 1, high);
    	}
	}
	bool ValToBool(int val)
	{
		if(val<0)
			return true;
		return false;
	}

	bool comp(const Clause *a,const Clause *b)
	{
		

			
		int smaller = ((a->atoms.size())>(b->atoms.size())?b->atoms.size():a->atoms.size());
		
		for(int j = 0;j<smaller;j++)
		{
			int val = (a->atoms[j]->predicate).compare(b->atoms[j]->predicate);
			
			if(val!=0)
			{
				return ValToBool(val);
			}
			
			int argSize = (a->atoms[j]->argList).size();
			for(int i = 0 ;i<argSize;i++)
			{
				val = TermCompare((a->atoms[j])->argList[i],(b->atoms[j])->argList[i]);
				if(val!=0)
				{
					return ValToBool(val);
				}
			}
			if(a->atoms[j]->isNeg!=b->atoms[j]->isNeg && a->atoms[j]->isNeg)
				val = -1;
			else if(a->atoms[j]->isNeg!=b->atoms[j]->isNeg && b->atoms[j]->isNeg)
				val = 1;
			if(val!=0)
			{
					return ValToBool(val);
			}
		 }
		if(a->atoms.size()>b->atoms.size())
			return false;
		else if(a->atoms.size()<b->atoms.size())
			return true;
		return true;
	//	return (RecCompare(a,b)<0?true:false);
	}
	bool operator()(const Clause *a,const Clause *b)
	{
		
	
			
		int smaller = ((a->atoms.size())>(b->atoms.size())?b->atoms.size():a->atoms.size());
		
		for(int j = 0;j<smaller;j++)
		{
			int val = (a->atoms[j]->predicate).compare(b->atoms[j]->predicate);
			
			if(val!=0)
			{
				return ValToBool(val);
			}
			
			int argSize = (a->atoms[j]->argList).size();
			for(int i = 0 ;i<argSize;i++)
			{
				val = TermCompare((a->atoms[j])->argList[i],(b->atoms[j])->argList[i]);
				if(val!=0)
				{
			//		printDebug(a,b,val);
					return ValToBool(val);
				}
			}
			if(a->atoms[j]->isNeg!=b->atoms[j]->isNeg && a->atoms[j]->isNeg)
				val = -1;
			else if(a->atoms[j]->isNeg!=b->atoms[j]->isNeg && b->atoms[j]->isNeg)
				val = 1;
			if(val!=0)
				{
					//printDebug(a,b,val);
					return ValToBool(val);
				}
		 }
		if(a->atoms.size()>b->atoms.size())
			return false;
		else if(a->atoms.size()<b->atoms.size())
			return true;
		return true;
	//	return (RecCompare(a,b)<0?true:false);
	}
	int TermCompare(const Term *a,const Term *b)
	{
		if(a->type == TermType::VARIABLE && b->type == TermType::VARIABLE)
		{
			return 0;
		}else
		if(a->type == TermType::VARIABLE && b->type == TermType::CONSTANT)
		{
			return 1;
		}else
		if(b->type == TermType::VARIABLE && a->type == TermType::CONSTANT)
		{
			return -1;
		}else
		{
			return (a->name).compare(b->name);
		}
	}
}clauseComparer;

struct ClauseEqualityComparer
{
	bool ValToBool(int val)
	{
		if(val<0)
			return true;
		return false;
	}
	int isEqual(const Clause *a,const Clause *b)
	{
		int smaller = (a->atoms.size()>b->atoms.size()?b->atoms.size():a->atoms.size());
		for(int j = 0;j<smaller;j++)
		{
			int val = (a->atoms[j]->predicate).compare(b->atoms[j]->predicate);
		
			if(val!=0)
				return val;
			int argSize = (a->atoms[j]->argList).size();
			for(int i = 0 ;i<argSize;i++)
			{
				val = TermCompare((a->atoms[j])->argList[i],(b->atoms[j])->argList[i]);
				if(val!=0)
					return val;
			}
			if(((a->atoms[j]->isNeg)!=(b->atoms[j]->isNeg ))&& (a->atoms[j]->isNeg))
				val = -1;
			else if(((a->atoms[j]->isNeg)!=(b->atoms[j]->isNeg)) && (b->atoms[j]->isNeg))
				val = 1;
			if(val!=0)
					return val;
		 }
		if((a->atoms).size()>(b->atoms).size())
			return 1;
		else if((a->atoms).size()<(b->atoms).size())
			return -1;
		return 0;
	//	return (RecCompare(a,b)<0?true:false);
	}
	int TermCompare(const Term *a,const Term *b)
	{
		if(a->type == TermType::VARIABLE && b->type == TermType::VARIABLE)
		{
			return 0;
		}else
		if(a->type == TermType::VARIABLE && b->type == TermType::CONSTANT)
		{
			return 1;
		}else
		if(b->type == TermType::VARIABLE && a->type == TermType::CONSTANT)
		{
			return -1;
		}else
		{
			return (a->name).compare(b->name);
		}
		//if(b.type == TermType::CONSTANT && a.type == TermType::CONSTANT)
	}
}clauseEqualityComparer;

typedef unordered_map<string,vector<Clause*> > FunctionClauseMapping;
vector<string> getLiteralFromSentence(string sentence)
{
	//cout<<"Printing sentence "<<sentence<<endl;
	sentence.erase(remove(sentence.begin(),sentence.end(),' '),sentence.end());
	vector<string> result;
	regex literalExp("[~]?[A-Z]+[a-zA-Z0-9]*[\\(][A-Za-z0-9, ]*[\\)]");
	smatch m;
	while(regex_search(sentence,m,literalExp))
	{
		
		for(auto x:m)
		{
	// 		cout<<x<<endl;
			result.push_back(x);
		}
 		sentence = m.suffix().str();
	}
	return result;
}
Clause* storeClause(vector<string> literals,FunctionClauseMapping &functionMapping)
{

	int n = literals.size();
	regex predicateExpr("[^~]?[A-Z]+[a-zA-Z0-9]*[\\(]");
	vector<Atom *> atoms;
	for(int i = 0;i<n;i++)
	{
		smatch m;
		string literal = literals[i];
		
		
		
		while(regex_search(literal,m,predicateExpr))
		{
			
			string match = m[0];
			string predicate = match.substr(0,match.size()-1);
			//cout<<predicate<<endl;
			bool isNeg = (literal[0]=='~'?true:false);

 			string argumentString = m.suffix().str();
			literal = m.suffix().str();

			argumentString = argumentString.substr(0,argumentString.size()-1); 
			argumentString = argumentString+",";
			
			//cout<<argumentString<<endl;
			vector<Term*> argList;
			regex argumentExpr("[A-Za-z0-9]{1,},{1}");
			smatch a;
			while(regex_search(argumentString,a,argumentExpr))
			{
				for(string x:a)
				{
					string termString = x.substr(0,x.size()-1);
					Term *term = new Term(termString);
					argList.push_back(term);
				}
 				argumentString = a.suffix().str();
			}
			Atom *atom = new Atom(predicate,argList,isNeg);
			atoms.push_back(atom);
		}
		
	}
	Clause *clause = new Clause(atoms);
	
	for(int i=0;i<atoms.size();i++)
	{
		//string key = atoms[i]->predicate;
		//if(functionMapping.find(key)==functionMapping.end())
			functionMapping[atoms[i]->predicate].push_back(clause);
	}
	return clause;

}
Clause* StoreQuery(string sentence)
{	
	vector<string> literals = getLiteralFromSentence(sentence);
	FunctionClauseMapping functionMapping;
	Clause *clause =storeClause(literals,functionMapping);	
	return clause;
}
Clause* Store(string sentence,FunctionClauseMapping &functionMapping)
{
	vector<string> literals = getLiteralFromSentence(sentence);
	Clause *clause =storeClause(literals,functionMapping);

	
	return clause;
}
bool Unify_VarConst(Term *t1,Term *t2,Theta& theta)
{
	if(theta.find(t1->name)!=theta.end() && theta[t1->name]!=(t2->name))
		return false;
	else if(theta.find(t1->name)!=theta.end() && theta[t1->name]==(t2->name))
		return true;
	else
	{
		theta[t1->name] = t2->name;
		return true;
	}
	
}
bool Unify(Atom *a1, Atom *a2,int i,int j,Theta& theta1,Theta& theta2)
{

//	cout<<i<<" "<<j<<endl;
	if(i==((a1->argList).size()) || j==((a2->argList).size()))
		return true;
	
	bool isPossible = Unify(a1,a2,i+1,j+1,theta1,theta2);
	if(!isPossible)
		return false;
	
	if((a1->argList[i]->type) == TermType::VARIABLE && (a2->argList[j]->type )== TermType::VARIABLE)
	{
		return false;
	}
	else if((a1->argList[i]->type )== TermType::VARIABLE)
	{
		return Unify_VarConst(a1->argList[i],a2->argList[j],theta2);
	}
	else if(a2->argList[j]->type == TermType::VARIABLE)
	{
		return Unify_VarConst(a2->argList[j],a1->argList[i],theta1);
	}
	else
	{
		if((a1->argList[i]->name )== (a2->argList[j]->name))
			return true;
		return false;
	}
	
	
}
void Substitute(Clause *query, Theta &theta)
{
	for(auto x:theta)
	{
		int numberOfAtoms = (query->atoms).size();
		for(int i =0;i<numberOfAtoms;i++)
		{
			int args = (query->atoms[i]->argList).size();
			for(int j=0;j<args;j++)
			{
				if(query->atoms[i]->argList[j]->name == x.first)
				{
					query->atoms[i]->argList[j]->name = x.second;
					query->atoms[i]->argList[j]->type = TermType::CONSTANT;
				}
			}
		}
	}
	sort((query->atoms).begin(),(query->atoms).end(),atomComparer);
	
}
void MergeClause(Clause *target,Clause *source,int indx)
{

	// Merges two clauses after resolution. Same Clauses check has to be added
	int numberOfAtoms = (source->atoms).size();
	for(int i =0;i<numberOfAtoms;i++)
	{
		if(i!=indx)
			(target->atoms).push_back(source->atoms[i]->copy());
			
	}
	target->isQuery = target->isQuery|| source->isQuery;
		//cout<<(target->atoms).size()<<endl;
		
}
void SubAndMerge(Clause *target,Clause *source,int indx,Theta theta1,Theta theta2)
{
	int numberOfAtoms = (source->atoms).size();
	int initialSize =(target->atoms).size();
	for(int i =0;i<numberOfAtoms;i++)
	{
		if(i!=indx)
			(target->atoms).push_back(source->atoms[i]->copy());
			
	}
	target->isQuery = target->isQuery|| source->isQuery;
	
	for(int i =0;i<initialSize;i++)
	{
		int args = (target->atoms[i]->argList).size();
		for(int j=0;j<args;j++)
		{
			if(theta1.find(target->atoms[i]->argList[j]->name)!=theta1.end())
			{
				target->atoms[i]->argList[j]->name = theta1[target->atoms[i]->argList[j]->name];
				target->atoms[i]->argList[j]->type = TermType::CONSTANT;
			}
		}
	}
	for(int i =initialSize;i<(target->atoms).size();i++)
	{
		int args = (target->atoms[i]->argList).size();
		for(int j=0;j<args;j++)
		{
			if(theta2.find(target->atoms[i]->argList[j]->name)!=theta2.end())
			{
				target->atoms[i]->argList[j]->name = theta2[target->atoms[i]->argList[j]->name];
				target->atoms[i]->argList[j]->type = TermType::CONSTANT;
			}
		}
	}
	sort((target->atoms).begin(),(target->atoms).end(),atomComparer);
}
bool Resolution(vector<Clause*> KB,Clause *query,FunctionClauseMapping functionMapping)
{
	//	cout<<"Called"<<endl;
	if((query->atoms).size()==0)
		return true;
	vector<Clause*> newList;

	//for each predicate in the query i.e. until resolution gives zero
	int m = 0;
	Clause *copyQuery = query->copy();
	while(m<(query->atoms).size() )
	{
		//possible clause to resolve with
		vector<Clause*> possibleClauses = functionMapping[query->atoms[m]->predicate];
		
		
		// try resolving with one of them
		int possibleQueries = possibleClauses.size();
		for(int i = 0;i<possibleQueries;i++)
		{
			
			Atom *funcInClause = NULL;
			// index of atom in clause i which we try to resolve
			//get function index to resolve with

			int indx = possibleClauses[i]->getAtomBinary(query->atoms[m]->predicate,!(query->atoms[m]->isNeg));
			// if found great else continue
			if(indx!=-1 )
				funcInClause= possibleClauses[i]->atoms[indx];
			else
				continue;

			//Try Unifying with found predicate in the clause
	
			Theta theta1;
			Theta theta2;
			bool isPossible = Unify(funcInClause,query->atoms[m],0,0,theta1,theta2);
			if(isPossible)
			{
				//then resolve by removing the query
				(query->atoms).erase((query->atoms).begin()+m);

			
				//Merge the literals from KB clause to query clause
				MergeClause(query,possibleClauses[i],indx);
				
				// susbtitue theta in query
				Substitute(query,theta1);
			
			
			}

			if(isPossible)
			{
				//printClause(query);
				if(!Resolution(KB,query->copy(),functionMapping))
					query = copyQuery->copy();
				else
					return true;
			}
			
		}
		m++;
	}
	if((query->atoms).size()!=0)
		return false;
	else
		return true;
}
bool HasEmptyClause(vector<Clause*> &newList,int prevSize)
{
	if(newList.size()>0 && (newList[0]->atoms).size()==0)
		return true;
	return false;
}
int CompareTree(Clause *t1,Clause *t2)
{
	//ClauseEqualityComparer clauseEqualityComparer1;
	return clauseEqualityComparer.isEqual(t1,t2);
}
bool recdifference(Clause *query,vector<Clause*> &KB)
{
	int low = 0;
	int high = KB.size()-1;
	while(low<=high)
	{
		int mid = low+(high-low)/2;
		int val = CompareTree(query,KB[mid]);
		if(val == 0)
			return true;
		if(val<0)
			high = mid-1;
		else
			low = mid+1;
	}
	return false;
}
bool difference1(Clause *query, vector<Clause*> &KB)
{
	return recdifference(query,KB);
}
bool difference(vector<Clause*> &newList, vector<Clause*> &KB)
{

	//checks is newList is subset of KB
	int newListLength = newList.size();
	int kbLength = KB.size();
	//int startIndex = -1;
	int endIndex = kbLength;//-1;
	//cout<<"Here"<<endl;
	int i = 0;
	int j = 0;
	int count = newListLength;
	while(i<newListLength && j<endIndex)
	{

		

			int val = CompareTree(newList[i],KB[j]);
		
			if(val>0)
				j++;
			else if(val<0)
				i++;
			else
			{
				Clause *temp = newList[i];
				deleteClause(temp);
				(newList).erase(newList.begin()+i);
				
				count--;
				newListLength--;
			
			}
	}
	//sort(newList.begin(),newList.end(),clauseComparer);
	if(count == 0)
		return true;
	
	//
	return false;
}

//vector<Clause*>
void Resolve(Clause *query,vector<Clause*> &result,vector<Clause*> &KB,int maxHeuristic,FunctionClauseMapping &functionMapping)
{
	int qlen = (query->atoms).size();
	
	Clause *copyQuery = query->copy();
	
	for(int m = 0 ; m<qlen;m++)
	{
		vector<Clause*> listOfClausesWithFunc = functionMapping[query->atoms[m]->predicate];
		for(int c = 0 ; c<listOfClausesWithFunc.size();c++)
		{
			Atom *funcInClause = NULL;

			int indx = listOfClausesWithFunc[c]->getAtomBinary(query->atoms[m]->predicate,!(query->atoms[m]->isNeg));
			// if found great else continue
			if(indx!=-1)
				funcInClause= (listOfClausesWithFunc[c])->atoms[indx];
			else
				continue;
			Theta theta1;
			Theta theta2;
			bool isPossible = false;
			isPossible = Unify(funcInClause,query->atoms[m],0,0,theta1,theta2);
			if(isPossible)
			{
				(query->atoms).erase((query->atoms).begin()+m);
				SubAndMerge(query,listOfClausesWithFunc[c],indx,theta1,theta2);
			}
			
			
			
			//cout<<i<<"Inside"<<possibleClauses.size()<<endl;
			if(isPossible)
			{
					if(((query->atoms).size())>=maxHeuristic)
						continue;
					vector<Clause*> queryList;
					queryList.push_back(query);

					
					bool isSubsetNewList = difference1(query,result);
					bool isSubsetKB = difference1(query,KB);

					if((!isSubsetNewList) && (!isSubsetKB))
					{
						if((query->atoms).size()!=0 || ((query->atoms).size()==0 && query->isQuery ))
						{
							result.push_back(query->copy());
							clauseComparer.quickSort(result,0,result.size()-1);
						}

						
					}
					query = copyQuery->copy();
	
			}
		}
	}
	

		
} 
// bool timeUp(chrono::system_clock::time_point start)
// {
// 		chrono::duration<double> elapsed_seconds=chrono::system_clock::now()-start;
		
// 		if(elapsed_seconds.count()>1000)
// 			return true;
// 		return false;
// }
bool Resolution1(vector<Clause*> KB,Clause *query,FunctionClauseMapping functionMapping,int maxHeuristic,chrono::system_clock::time_point start)
{
	chrono::system_clock::time_point start_inside=chrono::system_clock::now();
	//cout<<elapsed_seconds.count()<<endl;
	int intialKBSize = KB.size();
	while(1)
	{
	
		vector<Clause*> newList;
		int n = KB.size();
		for(int i = 0;i<(n-1);i++)
		{
				int prevSize = 0;
				Resolve(KB[i]->copy(),newList,KB,maxHeuristic,functionMapping);
				if(HasEmptyClause(newList,prevSize))
				{
					return true;
				}
				chrono::duration<double> elapsed_seconds_inside=chrono::system_clock::now()-start_inside;
				//cout<<elapsed_seconds_inside.count()<<endl;
				//timeUp(start) || 
				if(intialKBSize<15 && (elapsed_seconds_inside.count())> 30)
					return false;
				else
				if(intialKBSize<50 && (elapsed_seconds_inside.count())> 45)
					return false;
				else
				if(intialKBSize>50 && (elapsed_seconds_inside.count())> 50)
					return false;
				
		}
		for(int i = 0; i<newList.size();i++)
		{
			for(int j = 0;j<(newList[i]->atoms).size();j++)
			{
				functionMapping[newList[i]->atoms[j]->predicate].push_back(newList[i]);
			}
		}
		
		if(newList.size()==0)
		{
			return false;
		}
		KB.insert(KB.end() , newList.begin(), newList.end());
		clauseComparer.quickSort(KB,0,KB.size()-1);

	}
}
void AddToFunctionMapping(Clause *clause,FunctionClauseMapping &functionMapping)
{
	for(int i = 0;i<(clause->atoms).size();i++)
	{
		functionMapping[clause->atoms[i]->predicate].push_back(clause); 
	}
}
int main()
{
	auto start = chrono::system_clock::now();
	int n = 0;
	int q = 0;
	string sentence = "";
	vector<Clause*> KB;
	FunctionClauseMapping functionMapping;
	ifstream myfile ("input.txt");
	int maxHeuristic = 0;
	vector<Clause*> queryList;

	if (myfile.is_open())
  	{
		
		
		getline (myfile,sentence);
		stringstream pp(sentence);
				pp>>q;
		
		for(int i = 0;i<q;i++)
		{
			getline (myfile,sentence);
			Clause *temp = StoreQuery(sentence);
			temp->atoms[0]->isNeg = (!temp->atoms[0]->isNeg);
			temp->isQuery = true;
			queryList.push_back(temp);
		}
		getline (myfile,sentence);
		stringstream qq(sentence);
				qq>>n;
		while(n!=0)
		{
			getline(myfile,sentence);
			Clause *temp = Store(sentence,functionMapping);
			temp->isQuery = false;
			maxHeuristic = max(maxHeuristic,(temp->atoms).size());
			KB.push_back(temp);
			n--;
		}//while(n!=0);
	}
	vector<Clause*> KBCopy;
	ofstream outputFile;
	outputFile.open("output.txt");
	for(int i = 0;i<q;i++)
	{
		
		FunctionClauseMapping functionMapping1;
		for(int j = 0;j<KB.size();j++)
		{
			Clause *temp = KB[j]->copy();
			KBCopy.push_back(temp);
			AddToFunctionMapping(temp,functionMapping1);
		}
		
		KBCopy.push_back(queryList[i]);
		AddToFunctionMapping(queryList[i],functionMapping1);
		clauseComparer.quickSort(KBCopy,0,KBCopy.size()-1);
	
		bool succeed = Resolution1(KBCopy,queryList[i],functionMapping1,maxHeuristic,start);
		outputFile<<(succeed?"TRUE":"FALSE")<<endl;
		for(int j = 0;j<KBCopy.size();j++)
		{
			deleteClause(KBCopy[j]);
		}
		KBCopy.clear();
		
		//cout<<"Deleted"<<endl;
	}
	outputFile.close();
	return 0;
} 
