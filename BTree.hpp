#include "utility.hpp"
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <map>
#include <fstream>
#include <cstdio>
#include <cstring>
using namespace std;
namespace sjtu {
    template<class Key, class Value, class Compare = std::less<Key> >
    class BTree {

    private:
        // Your private members go here
        FILE *file;
        char name[50];

        static const int M = 228;
        static const int L = 32;

    public:
        struct Information {
            int headLeaf;
            int tailLeaf;
            size_t Size;
            int root;
            int end;

            Information() {
                headLeaf = tailLeaf = Size = root = end = 0;
            }
        };

        Information member;

        struct LeafNode {
            int offset;
            int parent;
            int prev;
            int next;
            int num;
            pair<Key, Value> data[L + 1];

            LeafNode(int t = 0) {
                offset = t;
                parent = prev = next = num = 0;
                memset(data, 0, L + 1);
            }
        };

        struct SimpleNode {
            int offset;
            int parent;
            int num;
            bool sonType;
            int son[M + 1];
            Key keyData[M + 1];

            SimpleNode(int t = 0) {
                parent = num = sonType = 0;
                offset = t;
                memset(son, 0, M + 1);
                memset(keyData, 0, M + 1);
            }
        };

        class const_iterator;

        class iterator {
            friend class BTree;

        private:
            // Your private members go here
            int offset;
            BTree *currentTree;

        public:
            bool modify(const Value &value) {

            }

            iterator() {
                // TODO Default Constructor
                currentTree = nullptr;
                offset = 0;
            }

            iterator(BTree *b, int p = 0) {
                currentTree = nullptr;
                offset = p;
            }

            iterator(const iterator &other) {
                // TODO Copy Constructor
                currentTree = other.currentTree;
                offset = other.offset;
            }

            // Return a new iterator which points to the n-next elements
            iterator operator++(int) {
                // Todo iterator++
            }

            iterator &operator++() {
                // Todo ++iterator
            }

            iterator operator--(int) {
                // Todo iterator--
            }

            iterator &operator--() {
                // Todo --iterator
            }

            // Overloaded of operator '==' and '!='
            // Check whether the iterators are same
            bool operator==(const iterator &rhs) const {
                // Todo operator ==
            }

            bool operator==(const const_iterator &rhs) const {
                // Todo operator ==
            }

            bool operator!=(const iterator &rhs) const {
                // Todo operator !=
            }

            bool operator!=(const const_iterator &rhs) const {
                // Todo operator !=
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            // Your private members go here
        public:
            const_iterator() {
                // TODO
            }

            const_iterator(const const_iterator &other) {
                // TODO
            }

            const_iterator(const iterator &other) {
                // TODO
            }
            // And other methods in iterator, please fill by yourself.
        };

        // Default Constructor and Copy Constructor
        BTree() {
            strcpy(name, "silly0.txt");
            file = fopen(name, "rb+");
            if(file == nullptr) {
                file = fopen(name, "wb+");
                member.Size = 0;
                member.end = sizeof(Information);
                SimpleNode root(member.end);
                root.num = 1, root.sonType = 1;
                member.root = root.offset;
                member.end += sizeof(SimpleNode);
                LeafNode leaf(member.end);
                member.headLeaf = member.tailLeaf = leaf.offset;
                member.end += sizeof(LeafNode);
                root.son[0] = leaf.offset;
                leaf.parent = root.offset;
                writeFile(&member, 0, 1, sizeof(Information));
                writeFile(&root, root.offset, 1, sizeof(SimpleNode));
                writeFile(&leaf, leaf.offset, 1, sizeof(LeafNode));
            } else {
                readFile(&member, 0, 1, sizeof(Information));
            }
            fflush(file);
        }

        BTree(const BTree &other) {}

        BTree &operator=(const BTree &other) {}

        ~BTree() {
            // Todo Destructor
            fclose(file);
        }

        void readFile(void *place, int offset, int num, int size) {
            fseek(file, offset, SEEK_SET);
            fread(place, num, size, file);
            fflush(file);
        }

        void writeFile(void *place, int offset, int num, int size) {
            fseek(file, offset, SEEK_SET);
            fwrite(place, num, size, file);
            fflush(file);
        }

        int searchLeaf(const Key &key, int offset) {
            SimpleNode p;
            readFile(&p, offset, 1, sizeof(SimpleNode));
            if (p.sonType) {
                int pos = 0;
                for (; pos < p.num; pos++)
                    if (key < p.keyData[pos]) break;
                if (pos == 0) return 0;
                return p.son[pos - 1];
            } else {
                int pos = 0;
                for (; pos < p.num; pos++)
                    if (key < p.keyData[pos]) break;
                if (pos == 0) return 0;
                return searchLeaf(key, p.son[pos - 1]);
            }
        }

        OperationResult insertInLeaf(LeafNode &leaf, const Key &key, const Value &value) {
            int pos = 0;
            for (; pos < leaf.num; pos++) {
                if (key == leaf.data[pos].first) return Fail;
                if (key < leaf.data[pos].first) break;
            }
            for (int i = leaf.num - 1; i >= pos; --i) {
                leaf.data[i + 1].first = leaf.data[i].first;
                leaf.data[i + 1].second = leaf.data[i].second;
            }
            leaf.data[pos].first = key, leaf.data[pos].second = value;
            leaf.num++;
            member.Size++;
            //leaf.offset = member.end;
            writeFile(&member, 0, 1, sizeof(Information));
            if (leaf.num <= L) writeFile(&leaf, leaf.offset, 1, sizeof(LeafNode));
            else splitLeaf(leaf, key);
            fflush(file);
            return Success;
        }

        void splitLeaf(LeafNode &leaf, const Key &key) {
            LeafNode newLeaf;
            newLeaf.num = leaf.num / 2;
            leaf.num -= newLeaf.num;
            newLeaf.offset = member.end;
            member.end += sizeof(LeafNode);
            newLeaf.parent = leaf.parent;
            for (int i = 0; i < newLeaf.num; i++) {
                newLeaf.data[i].first = leaf.data[i + leaf.num].first;
                newLeaf.data[i].second = leaf.data[i + leaf.num].second;
            }
            newLeaf.next = leaf.next;
            newLeaf.prev = leaf.offset;
            leaf.next = newLeaf.offset;
            LeafNode nextLeaf;
            if (newLeaf.next != 0) {
                readFile(&nextLeaf, newLeaf.next, 1, sizeof(LeafNode));
                nextLeaf.prev = newLeaf.offset;
                writeFile(&nextLeaf, nextLeaf.offset, 1, sizeof(LeafNode));
            }//update next
            if (member.tailLeaf == leaf.offset) member.tailLeaf = newLeaf.offset; // split tail
            writeFile(&leaf, leaf.offset, 1, sizeof(LeafNode));
            writeFile(&newLeaf, newLeaf.offset, 1, sizeof(LeafNode));
            writeFile(&member, 0, 1, sizeof(Information));
            SimpleNode parent;
            readFile(&parent, leaf.parent, 1, sizeof(SimpleNode));
            insertInNode(parent, newLeaf.data[0].first, newLeaf.offset);
            fflush(file);
        }

        void insertInNode(SimpleNode &node, const Key &key, int newSon) {
            int pos = 0;
            for (; pos < node.num; pos++)
                if (key < node.keyData[pos]) break;
            for (int i = node.num - 1; i >= pos; i--)
                node.keyData[i + 1] = node.keyData[i];
            for (int i = node.num - 1; i >= pos; i--)
                node.son[i + 1] = node.son[i];
            node.keyData[pos] = key;
            node.son[pos] = newSon;
            node.num++;
            if (node.num <= M) writeFile(&node, node.offset, 1, sizeof(SimpleNode));
            else splitNode(node);
            fflush(file);
        }

        void changeRoot(SimpleNode &node, SimpleNode &newNode) {
            SimpleNode newRoot;
            newRoot.parent = newRoot.sonType = 0;
            newRoot.offset = member.end;
            member.end += sizeof(SimpleNode);
            newRoot.num = 2;
            newRoot.keyData[0] = node.keyData[0];
            newRoot.keyData[1] = newNode.keyData[0];
            newRoot.son[0] = node.offset;
            newRoot.son[1] = newNode.offset;
            member.root = node.parent = newNode.parent = newRoot.offset;
            writeFile(&member, 0, 1, sizeof(Information));
            writeFile(&node, node.offset, 1, sizeof(SimpleNode));
            writeFile(&newNode, newNode.offset, 1, sizeof(SimpleNode));
            writeFile(&newRoot, newRoot.offset, 1, sizeof(SimpleNode));
        }

        void splitNode(SimpleNode &node) {
            SimpleNode newNode;
            newNode.num = node.num / 2;
            node.num -= newNode.num;
            newNode.parent = node.parent;
            newNode.sonType = node.sonType;
            newNode.offset = member.end;
            member.end += sizeof(SimpleNode);
            for (int i = 0; i < newNode.num; ++i) {
                newNode.son[i] = node.son[node.num + i];
                newNode.keyData[i] = node.keyData[node.num + i];
            }
            LeafNode leaf;
            SimpleNode tmp;
            for (int j = 0; j < newNode.num; ++j) {
                if (newNode.sonType) {
                    readFile(&leaf, newNode.son[j], 1, sizeof(LeafNode));
                    leaf.parent = newNode.offset;
                    writeFile(&leaf, leaf.offset, 1, sizeof(LeafNode));
                } else {
                    readFile(&tmp, newNode.son[j], 1, sizeof(SimpleNode));
                    tmp.parent = newNode.offset;
                    writeFile(&tmp, tmp.offset, 1, sizeof(SimpleNode));
                }
            }
            if (node.offset == member.root)
                changeRoot(node, newNode);
            else {
                writeFile(&member, 0, 1, sizeof(Information));
                writeFile(&node, node.offset, 1, sizeof(SimpleNode));
                writeFile(&newNode, newNode.offset, 1, sizeof(SimpleNode));
                SimpleNode parent;
                readFile(&parent, node.parent, 1, sizeof(SimpleNode));
                insertInNode(parent, newNode.keyData[0], newNode.offset);
            }
            fflush(file);
        }

        OperationResult insertMin(LeafNode &leaf, int leafOffset, const Key &key, const Value &value) {
            readFile(&leaf, member.headLeaf, 1, sizeof(LeafNode));
            OperationResult t = insertInLeaf(leaf, key, value);
            if (t == Fail) return t;
            int offset = leaf.parent;
            SimpleNode node;
            while (offset != 0) {
                readFile(&node, offset, 1, sizeof(SimpleNode));
                node.keyData[0] = key;
                writeFile(&node, offset, 1, sizeof(SimpleNode));
                offset = node.parent;
            }
            fflush(file);
            return t;
        }

        pair<iterator, OperationResult> insert(const Key &key, const Value &value) {
            int leafOffset = searchLeaf(key, member.root);
            LeafNode leaf;
            if (member.Size == 0 || leafOffset == 0) {
                OperationResult t = insertMin(leaf, leafOffset, key, value);
                return pair<iterator, OperationResult>(iterator(nullptr), t);
            }//min or empty
            readFile(&leaf, leafOffset, 1, sizeof(LeafNode));
            OperationResult t = insertInLeaf(leaf, key, value);
            fflush(file);
            return pair<iterator, OperationResult>(iterator(nullptr), t);
        }

        // Erase: Erase the Key-Value
        // Return Success if it is successfully erased
        // Return Fail if the key doesn't exist in the database
        OperationResult erase(const Key &key) {
            // TODO erase function
            return Fail;  // If you can't finish erase part, just remaining here.
        }

        bool empty() const {
            return !member.Size;
        }

        // Return the number of <K,V> pairs
        size_t size() const {
            return member.Size;
        }

        Value at(const Key &key) {
            int leafLocation = searchLeaf(key, member.root);
            if (leafLocation == 0) throw "error";
            LeafNode leaf;
            readFile(&leaf, leafLocation, 1, sizeof(LeafNode));
            for (int i = 0; i < leaf.num; i++)
                if (leaf.data[i].first == key) return leaf.data[i].second;
            throw "error";
        }
    };
} // namespace sjtu
