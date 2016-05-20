#include<iostream>
//#include "CoverTree.cpp"
//#include "Space.cpp"
#include <map>
#include <vector>
#include <set>
#include <ctime> 
using namespace std;

const double MAX_LEVEL = ceilf(log(INT_MAX) / log(2.0));
const double MIN_LEVEL = ceilf(log(FLT_MIN) / log(2.0));

using std::string;

class Object
{
	double value;
public:
	Object(double _value)
	{
		value = _value;
	}

	double get_value() const
	{
		return value;
	}
};


template <typename dist_t>
class KNNQuery {
	Object* query;
	int k;
	int DistCount = 0;
	vector<const Object*>* objectVector;
	vector<dist_t>* distVector;
public:
	KNNQuery(int _k, Object* _query)
	{
		k = _k;
		query = _query;
		objectVector = new vector<const Object*>;
		distVector = new vector<dist_t>;
	}

	int get_k()
	{
		return k;
	}
	vector<double> get_Points()
	{
		vector<double> toReturn;
		for (int i = 0; i < objectVector->size(); i++)
		{
			toReturn.push_back((*objectVector)[i]->get_value());
		}
		return toReturn;
	}

	vector<const Object*> get_Objects()
	{
		return *objectVector;
	}
	vector<dist_t> get_Dists()
	{
		return *distVector;
	}
	dist_t DistanceObjLeft(const Object* object)
	{
		DistCount++;
		return abs(query->get_value() - object->get_value());
	}

	void CheckAndAddToResult(const dist_t dist, const Object* object)
	{
		distVector->push_back(dist);
		objectVector->push_back(object);
	}
	void CheckAndAddToResult(const Object* object)
	{
		objectVector->push_back(object);
	}
	void CheckAndAddToResult(const vector<Object*>& bucket)
	{
		objectVector->insert(objectVector->end(), bucket.begin(), bucket.end());
	}
};
template <typename dist_t>
class Space
{
public:
	Space()
	{}
	dist_t IndexTimeDistance(const Object* obj1, const Object* obj2)
	{
		return abs(obj1->get_value() - obj2->get_value());
	}
};
template <typename dist_t>
class CoverTreeMethod
{
private:

	class CoverTreeNode {
	private:
		map<int, vector<CoverTreeNode*>> children;
		//Object* object;
		vector<const Object*> objectVector;
	public:
		CoverTreeNode(const Object* _object)
		{
			//object = _object;
			//vector<const Object*>objectVector;
			objectVector.push_back(_object);
		}
		const Object* get_object()
		{
			return objectVector[0];
		}
		vector<const Object*> get_all()
		{
			return objectVector;
		}
		vector<CoverTreeNode*> get_children(int level)
		{
			map<int, vector<CoverTreeNode*>>::iterator it = children.find(level);
			if (it == children.end())
			{
				return vector <CoverTreeNode*>();
			}
			return children.at(level);
		}
		void insert_child(int level, CoverTreeNode* child)
		{
			children[level].push_back(child);
		}

		void insert_clone(CoverTreeNode* clone)
		{
			objectVector.push_back(clone->get_object());
		}
	};//CoverTreeNode
	CoverTreeNode* root;
public:
	CoverTreeMethod(Space<dist_t>* space,
		const vector<Object*> data)
	{
		if (root == NULL)
		{
			root = new CoverTreeNode(data[0]);
		}
		for (int i = 1; i < data.size(); i++)
		{
			vector<CoverTreeNode*> tmp = { root };
			insert(space, new CoverTreeNode(data[i]), tmp, MAX_LEVEL);
		}
	}

	bool insert(Space<dist_t>* space, CoverTreeNode* point, vector<CoverTreeNode*>& level_points, int level)
	{
		vector<CoverTreeNode*> children = level_points;
		for (int i = 0; i < level_points.size(); i++)
		{
			vector<CoverTreeNode*> tmp = level_points[i]->get_children(level);
			children.insert(children.end(), tmp.begin(), tmp.end());
		}
		vector<CoverTreeNode*> parents;
		for (int i = 0; i < children.size(); i++)
		{
			double check = pow(2, level);
			if (!(space->IndexTimeDistance(point->get_object(), children[i]->get_object()) >= check))//...			
			{
				if (space->IndexTimeDistance(point->get_object(), children[i]->get_object()) == 0)
				{
					children[i]->insert_clone(point);
					return false;
				}
				else {
					CoverTreeNode* tmp = children[i];
					parents.push_back(tmp);
				}
			}
		}
		if (parents.empty())
		{
			return true;
		}
		else
		{
			bool found = insert(space, point, parents, level - 1);
			if (found)
			{
				parents[0]->insert_child(level, point);
				return false;
			}
			else
			{
				return found;
			}
		}
	}


	void Search(KNNQuery<dist_t>* query)
	{
		int k = query->get_k();
		vector<pair<dist_t, CoverTreeNode*>> level_points = { make_pair(query->DistanceObjLeft(root->get_object()), root) };
		set<pair<dist_t, CoverTreeNode*>>answer;
		answer.insert(level_points[0]);
		dist_t minDist = level_points[0].first;//дл€ работы алгоритма
		dist_t maxDist = level_points[0].first;//дл€ составлени€ ответа
		for (int level = MAX_LEVEL; level >= MIN_LEVEL; level--)
		{
			for (int j = 0; j < level_points.size(); j++)
			{
				vector<CoverTreeNode*> children = level_points[j].second->get_children(level);
				for (vector<CoverTreeNode*>::iterator i = children.begin(); i != children.end(); i++)
				{
					dist_t dist = query->DistanceObjLeft((*i)->get_object());
					if (dist <= minDist)
					{
						minDist = dist;
					}
					if (dist < maxDist || answer.size() < k)
					{
						//answer.push_back((dist, children[i]));
						answer.insert(std::make_pair(dist, *i));
						//--answer.end() gives us an iterator to the greatest
						//element of minNodes.
						if (answer.size() > k) answer.erase(--answer.end());
						maxDist = (--answer.end())->first;
					}
					level_points.push_back(make_pair(dist, *i));
				}
			}
			dist_t check = pow(2, level) + minDist;
			int size = level_points.size();
			for (int i = 0; i<size; i++)
			{
				if (level_points[i].first > check)//дистанци€ считаетс€ второй раз. ћожетстоило еЄ сохранить?
				{
					level_points.erase(level_points.begin() + i);
					size--; i--;
				}
			}
		}
		//set<pair<dist_t, CoverTreeNode*>>::iterator it
		for (set<pair<dist_t, CoverTreeNode*>>::iterator it = answer.begin(); it != answer.end(); it++)//....
		{
			//CoverTreeNode* a= answer[it].second;
			vector<const Object*> objectVector = it->second->get_all();
			for (int i = 0; i < objectVector.size(); i++)
			{
				query->CheckAndAddToResult(it->first, objectVector[i]);
			}
		}
	}

	/*void Search(KNNQuery<dist_t>* query)
	{

	}*/
};//CoverTree

void test(int N, int K)//N is amount of objects in tree; K is required amound of neighbours
{
	cout << "Test with " << N << " objects begins" << endl;
	Object* search_object = new Object(rand());
	KNNQuery<double>* query = new KNNQuery<double>(K, search_object);//второй аргумент точка, к которой ищем ближайших соседей; первый аргумент - их количество
	Space<double>* fakespace = new Space<double>();
	vector<Object*> data;
	multiset<pair<double, const Object*>>data_set;//<dist,Object>
	multiset<pair<double, const Object*>>check;
	//vector<Object*> data = {new Object(1), new Object(5), new Object(6),  new Object(6), new Object(7), new Object(46), new Object(3), new Object(2) };
	for (int i = 0; i < N; i++)
	{
		Object* r = new Object(rand());
		data.push_back(r);
		data_set.insert(make_pair(fakespace->IndexTimeDistance(r, search_object), r));
	}
	multiset<pair<double, const Object*>>::iterator it = data_set.begin();
	for (int i = 0; i < query->get_k(); i++)
	{
		if (it == data_set.end())
		{
			cout << "ERROR:K is bigger than data.size()" << endl;
			break;
		}
		check.insert(*it);
		it++;
	}
	cout << "Data set created..." << endl;

	unsigned int start_tree_creation_time = clock();
	CoverTreeMethod<double> a(fakespace, data);
	unsigned int end_tree_creation_time = clock();
	cout << "Tree creation time: " << end_tree_creation_time - start_tree_creation_time << endl;

	unsigned int start_search_time = clock();
	a.Search(query);
	unsigned int end_search_time = clock();
	cout << "Search time: " << end_search_time - start_search_time << endl;

	vector<const Object*> result = query->get_Objects();
	vector<double> distResult = query->get_Dists();
	int right_answer_count = 0;
	int i = 0;
	//for (multiset<pair<double, Object*>>::iterator it = check.begin(); i < query->get_k(); it++)//тк элементы могут идти в разном пор€дке - не идеально
	//{
	// if (i >= result.size())
	// {
	//	 cout << "search returned less objects than k" << endl;
	//	 break;
	// }
	// if (it->second == result[i])
	// {
	//	 right_answer_count++;
	// }
	// i++;
	//}
	//vector<const Object*>::iterator it = result.begin(); it != result.end(); it++
	for (i; i < query->get_k(); i++)
	{
		if (i >= result.size())
		{
			cout << "ERROR:Search returned less objects than k" << endl;
			break;
		}
		multiset<pair<double, const Object*>>::iterator it2 = check.find(make_pair(distResult[i], result[i]));
		if (it2 != check.end())
		{
			right_answer_count++;
		}
	}
	cout << "Right neighbours " << right_answer_count << endl;
	cout << "From k=" << query->get_k() << endl;
	cout << "Percentage of right answers: " << (float)right_answer_count / i << endl;
}
int main()
{
	test(100, 30);//first argument is amount of objects in tree; second is required amound of neighbours
	test(10000, 30);
	test(100000, 30);
	system("pause");

}