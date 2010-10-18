/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

#include "hbsharedmemoryallocators_p.h"

#include <QSharedMemory>
#include <QDebug>

#define TO_PTR(x) ((char *)(toPointer(x)))
#define TO_NODE_POINTER(x) ((TreeNode *)(toPointer(x)))
#define TO_BLOCK_POINTER(x) ((MemoryBlock *)(toPointer(x)))
#define TO_NODE_OFFSET(x) ((int)((char *)(x)-(char *)(chunk->data())))
#define TO_OFFSET(x) ((int)((char *)(x)-(char *)(chunk->data())))

// this identifier is used to check, if the splay tree is already
// initialized in given shared chunk
const quint32 INITIALIZED_ALLOCATOR_IDENTIFIER = 0x53504C59; //'SPLY'

/**
 * HbSplayTreeAllocator::initialize
 *
 * Initializes splay tree and internal variables.
 * This can't fail if sharedChunk is valid.
 */
void HbSplayTreeAllocator::initialize(HbSharedMemoryWrapper *sharedChunk,
                                      const quintptr offset,
                                      HbSharedMemoryAllocator *mainAllocator)
{
    Q_UNUSED(mainAllocator);
    chunk = sharedChunk;
    this->offset = offset;

    header = address<HeapHeader>(offset);
    if (header->identifier == INITIALIZED_ALLOCATOR_IDENTIFIER) {
        return; // already initialized
    }

    memset(header, 0, sizeof(HeapHeader));
    header->freeBytes = chunk->size() - offset - sizeof(HeapHeader) - sizeof(MemoryBlock);

    // insert first memory block in chunk
    MemoryBlock *block = reinterpret_cast<MemoryBlock*>(&header[1]);

    block->pointerNode.key = TO_OFFSET(block);
    block->lengthNode.key = static_cast<quintptr>(header->freeBytes);

    insertLengthNode(&header->lengthNode, &block->lengthNode);
    insertNode(&header->pointerNode, &block->pointerNode);

    header->identifier = INITIALIZED_ALLOCATOR_IDENTIFIER;
}

// Constructor
//
HbSplayTreeAllocator::HbSplayTreeAllocator()
    : chunk(0),
      offset(0)
{
}

// Destructor
//
HbSplayTreeAllocator::~HbSplayTreeAllocator()
{
}

/**
* HbSplayTreeAllocator::toPointer
*
* Helper function for converting offset to pointer
*/
void *HbSplayTreeAllocator::toPointer(quintptr offset) const
{
    if (offset >= sizeof(HeapHeader)) {
        unsigned char *base = static_cast<unsigned char*>(chunk->data());
        return (&base[offset]);
    }
    return 0;
}

/**
 * HbSplayTreeAllocator::alloc
 * 
 * Allocate size bytes from shared memory.
 * Returns offset of allocated memory in shared chunk
 * or -1 if allocation failed.
 */
qptrdiff HbSplayTreeAllocator::alloc(int size)
{
    if (size <= 0) {
        throw std::bad_alloc();
    }

    size = ALIGN(size);

    if (size > int(header->freeBytes - sizeof(MemoryBlock))) {
        throw std::bad_alloc();
    }

    // splay the 'length' tree to obtain the best match
    TreeNode *node = TO_NODE_POINTER(splay(&header->lengthNode, static_cast<quintptr>(size)));
    bool splayed = true;

    if (node && (node->key < static_cast<quintptr>(size))) {
        splayed = false;
        node = TO_NODE_POINTER(node->rightNode);
        if (node) {
            while (node->leftNode) {
                node = TO_NODE_POINTER(node->leftNode);
            }
        }
    }

    // no big enough nodes found, return error
    if (!node) {
        throw std::bad_alloc();
    }

    // 'length' node is the first attribute of the MemoryBlock, therefore every
    // 'length' node is also a MemoryBlock.
    MemoryBlock *block = reinterpret_cast<MemoryBlock*>(node);
    int totalSize = size + sizeof(MemoryBlock);
    int remainingSize = int(node->key) - totalSize;
    unsigned char *ptr = reinterpret_cast<unsigned char*>(&block[1]);

    // Remove current block from the trees
    deleteLengthNode(&header->lengthNode, node, splayed);
    deleteNode(&header->pointerNode, &block->pointerNode, false);

    // If the block is larger than the requested size, split the block and 
    // insert the second block into the trees
    MemoryBlock *secondBlock = reinterpret_cast<MemoryBlock*>(&ptr[size]);
    if (remainingSize >= ALIGN_SIZE
        && reinterpret_cast<unsigned char*>(secondBlock)
           < address<unsigned char>(chunk->size() - sizeof(MemoryBlock))) {

        block->lengthNode.key = static_cast<quintptr>(size);
        secondBlock->lengthNode.key = static_cast<quintptr>(remainingSize);
        secondBlock->pointerNode.key = TO_NODE_OFFSET(secondBlock);

        // Insert the second block into the trees
        insertLengthNode(&header->lengthNode, &secondBlock->lengthNode);
        insertNode(&header->pointerNode, &secondBlock->pointerNode);
    } else {
        totalSize = int(node->key);
    }

    // Adjust the allocation size
    header->freeBytes -= totalSize;
    header->allocatedBytes += totalSize;

    // Clear attributes and return the pointer
    block->lengthNode.leftNode = 0;
    block->lengthNode.rightNode = 0;
    block->pointerNode.leftNode = 0;
    block->pointerNode.rightNode = 0;
    block->next = 0;
    block->prev = 0;
    block->allocatorIdentifier = MAIN_ALLOCATOR_IDENTIFIER;

    return TO_OFFSET(ptr);
}

/**
 * HbSplayTreeAllocator::allocatedSize
 *
 * Used for reallocation.
 * Returns actual allocated size for given offset.
 */
int HbSplayTreeAllocator::allocatedSize(qptrdiff offset)
{
    int size = 0;
    if (offset > 0) {
        char *srcPtr = TO_PTR(offset);
        MemoryBlock *block = reinterpret_cast<MemoryBlock*>(srcPtr - sizeof(MemoryBlock));
        size = block->lengthNode.key;
    }
    return size;
}

/**
 * HbSplayTreeAllocator::free
 *
 * Releases memory by pushing MemoryBlock's nodes back into the trees.
 * Also does adjacent free blocks merging.
 */
void HbSplayTreeAllocator::free(qptrdiff offset)
{
    if (offset <= 0) {
        return;
    }

    MemoryBlock *block = TO_BLOCK_POINTER(offset - sizeof(MemoryBlock));
    MemoryBlock *nextBlock = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(&block[1]) + block->lengthNode.key);

    // Adjust the free bytes
    header->freeBytes += block->lengthNode.key;
    header->allocatedBytes -= block->lengthNode.key;

    TreeNode *node = 0;
    quintptr predecessor = 0;
    if (reinterpret_cast<char*>(nextBlock) > reinterpret_cast<char*>(chunk->data())
        && reinterpret_cast<char*>(nextBlock) < address<char>(chunk->size())) {
        node = TO_NODE_POINTER(splay(&header->pointerNode, nextBlock->pointerNode.key));
    }

    if (node) {
        // Find previous block
        if (node->key < nextBlock->pointerNode.key) {
            predecessor = TO_NODE_OFFSET(node);
        } else if ((predecessor = node->leftNode) > 0) {
            while (TO_NODE_POINTER(predecessor)->rightNode) {
                predecessor = TO_NODE_POINTER(predecessor)->rightNode;
            }
        }

        // See if the block can be merged with the sucessor
        if (node->key == nextBlock->pointerNode.key) {
            // Remove the successor from both trees
            deleteLengthNode(&header->lengthNode, &nextBlock->lengthNode, false);
            deleteNode(&header->pointerNode, &nextBlock->pointerNode, true); 

            // Adjust the size
            block->lengthNode.key += (nextBlock->lengthNode.key + sizeof(MemoryBlock));
            header->freeBytes += sizeof(MemoryBlock);
            header->allocatedBytes -= sizeof(MemoryBlock);
        }

        // See if the block can be merged with the predecessor
        if (predecessor) {
            MemoryBlock *b = TO_BLOCK_POINTER(TO_NODE_POINTER(predecessor)->key);
            MemoryBlock *t = reinterpret_cast<MemoryBlock*>(
                                reinterpret_cast<unsigned char*>(&b[1]) + b->lengthNode.key);

            // Merge with the predecessor
            if (t->pointerNode.key == block->pointerNode.key) {
                // Remove the predecessor from the 'length' tree
                deleteLengthNode(&header->lengthNode, &b->lengthNode, false);

                // Adjust the size
                b->lengthNode.key += block->lengthNode.key + sizeof(MemoryBlock);
                header->freeBytes += sizeof(MemoryBlock);
                header->allocatedBytes -= sizeof(MemoryBlock);

                // Re-insert the node in 'length' tree
                insertLengthNode(&header->lengthNode, &b->lengthNode);
                block = 0; // We don't have to insert the node
            }
        }
    }

    // Insert the node
    if (block) {
        insertLengthNode(&header->lengthNode, &block->lengthNode);
        insertNode(&header->pointerNode, &block->pointerNode);
    }
}

/**
 * HbSplayTreeAllocator::splay
 * 
 * Adjust the tree so that the best matching node goes to the top.
 */
quintptr HbSplayTreeAllocator::splay(quintptr *root, quintptr key)
{
    TreeNode *t = TO_NODE_POINTER(*root);
    if (!t) {
        return 0;
    }

    TreeNode n;
    n.key = 0;
    n.leftNode = 0;
    n.rightNode = 0;
    TreeNode *left = &n;
    TreeNode *right = left;
    TreeNode *temp = 0;

    for (;;) {
        if (key < t->key) {
            if (t->leftNode == 0) {
                break;
            }

            // Rotate right
            if (key < TO_NODE_POINTER(t->leftNode)->key) {
                temp = TO_NODE_POINTER(t->leftNode);
                t->leftNode = temp->rightNode;
                temp->rightNode = TO_NODE_OFFSET(t);
                t = temp;

                if (t->leftNode == 0) {
                    break;
                }
            }

            // Link right
            right->leftNode = TO_NODE_OFFSET(t);
            right = t;
            t = TO_NODE_POINTER(t->leftNode);
        } else if (key > t->key) {
            if (t->rightNode == 0) {
                break;
            }

            // Rotate left
            if (key > TO_NODE_POINTER(t->rightNode)->key) {
                temp = TO_NODE_POINTER(t->rightNode);
                t->rightNode = temp->leftNode;
                temp->leftNode = TO_NODE_OFFSET(t);
                t = temp;

                if (t->rightNode == 0) {
                    break;
                }
            }
                              
            // Link left
            left->rightNode = TO_NODE_OFFSET(t);
            left = t;
            t = TO_NODE_POINTER(t->rightNode);
        } else {
            break;
        }
    }

    // Re-create the node
    left->rightNode = t->leftNode;
    right->leftNode = t->rightNode;
    t->leftNode = n.rightNode;
    t->rightNode = n.leftNode;

    return (*root = TO_NODE_OFFSET(t));
}

/**
 * HbSplayTreeAllocator::deleteNode
 * 
 * Delete a node from the tree. If 'splayed' is true, then the tree
 * is assumed to be splayed with the correct key.
 */
void HbSplayTreeAllocator::deleteNode(quintptr *root, TreeNode *node, bool splayed)
{
    quintptr x = *root;

    if (splayed) {
        x = TO_NODE_OFFSET(node);
    } else {
        x = splay(&x, node->key);
    }

    if (TO_NODE_POINTER(x)->key == node->key) {
        if (node->leftNode == 0) {
            x = node->rightNode;
        } else {
            x = splay(&node->leftNode, node->key);
            TO_NODE_POINTER(x)->rightNode = node->rightNode;
        }
    }
    *root = x;
}

/**
 * HbSplayTreeAllocator::insertNode
 * 
 * Insert a new node into the tree.
 */
void HbSplayTreeAllocator::insertNode(quintptr *root, TreeNode *node)
{
    insertNode(root, node, TO_NODE_POINTER(splay(root, node->key)));
}

/**
 * HbSplayTreeAllocator::insertNode
 * 
 * Insert a new node into the tree.
 */
void HbSplayTreeAllocator::insertNode(quintptr *root, TreeNode *node, TreeNode *t)
{
    if (t == 0) {
        node->leftNode = 0;
        node->rightNode = 0;
    } else if (node->key < t->key) {
        node->leftNode = t->leftNode;
        node->rightNode = TO_NODE_OFFSET(t);
        t->leftNode = 0;
    } else {
        node->rightNode = t->rightNode;
        node->leftNode = TO_NODE_OFFSET(t);
        t->rightNode = 0;
    }

    *root = TO_NODE_OFFSET(node);
}

/**
 * HbSplayTreeAllocator::deleteLengthNode
 * 
 * For length nodes, we must additionally check linked list for correct node.
 */
void HbSplayTreeAllocator::deleteLengthNode(quintptr *root, TreeNode *node, bool splayed)
{
    MemoryBlock *block = reinterpret_cast<MemoryBlock*>(node);

    // If the node is not the first node in the linked list,
    // simply de-link node from the linked list.
    if (block->prev) {
        TO_BLOCK_POINTER(block->prev)->next = block->next;
        if (block->next) {
            TO_BLOCK_POINTER (block->next)->prev = block->prev;
        }
    } else {
        // If the node is the first node of the linked list, then
        // remove it from the tree and add the next node from the linked
        // list to the tree.
        if (block->next) {
            MemoryBlock *t = TO_BLOCK_POINTER(block->next);
            quintptr x = *root;
            
            if (splayed) {
                x = TO_NODE_OFFSET(node);
            } else {
                x = splay(&x, node->key);
            }

            if (x && TO_NODE_POINTER(x)->key == node->key) {
                t->lengthNode.leftNode = TO_NODE_POINTER(x)->leftNode;
                t->lengthNode.rightNode = TO_NODE_POINTER(x)->rightNode;
                t->prev = 0;
                *root = static_cast<quintptr>(block->next);
            }
        } else {
            deleteNode(root, node, false);
        }
    }
}

/**
 * HbSplayTreeAllocator::insertLengthNode
 * 
 * Many length nodes can have the same key. Therefore, we have to build
 * linked list for the nodes with same key.
 */
void HbSplayTreeAllocator::insertLengthNode(quintptr *root, TreeNode *node)
{
    // Length TreeNode is the first entry in the MemoryBlock. Therefore,
    // we can safely typecast TreeNode to a MemoryBlock.
    MemoryBlock *t = TO_BLOCK_POINTER(splay(root, node->key));
    MemoryBlock *p = reinterpret_cast<MemoryBlock*>(node);

    if (!t || (t->lengthNode.key != node->key)) {
        // Insert into the tree
        p->prev = 0;
        p->next = 0;
        insertNode(root, node, reinterpret_cast<TreeNode*>(t));
    } else {
        // Link to the existing tree node
        p->next = t->next;
        p->prev = TO_NODE_OFFSET(t);
        t->next = TO_NODE_OFFSET(p);
        
        if (p->next) {
            TO_BLOCK_POINTER(p->next)->prev = TO_NODE_OFFSET(p);
        }
    }
}

/**
 * HbSplayTreeAllocator::size
 *
 * Gets the total size as bytes used.
 */
int HbSplayTreeAllocator::size()
{
    // splay the 'pointer' tree to obtain last pointer
    quintptr last_offset = static_cast<quintptr>(chunk->size());
    TreeNode *node = TO_NODE_POINTER(splay(&header->pointerNode, last_offset));

    if (node) {
        TreeNode *right = TO_NODE_POINTER(node->rightNode);
        if (right) {
            node = right;
        }
        MemoryBlock *block = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(node)
                             - sizeof(TreeNode));
        return TO_OFFSET(block) + sizeof(MemoryBlock) - 1; // not aligned, but actual last byte allocated.;
    }
    return -1;
}

int HbSplayTreeAllocator::freeBytes()
{
    return header->freeBytes;
}

int HbSplayTreeAllocator::allocatedBytes()
{
    return header->allocatedBytes;
}

#ifdef HB_THEME_SERVER_MEMORY_REPORT
void HbSplayTreeAllocator::writeReport(QTextStream &reportWriter)
{
    reportWriter << "***** (Main)HbSplayTreeAllocator report *****\n\n";
    reportWriter << "Allocated memory (including memory allocated by multisegment allocator): "
                 << header->allocatedBytes << " bytes\n";
    reportWriter << "Free memory: " << header->freeBytes << " bytes\n";
    reportWriter << "Splaytree allocator is best fit allocator, so there is really no point to calculate fragmentation\n\n";
}
#endif
