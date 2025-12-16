#ifndef CREATE_DUMP_FILES_H
#define CREATE_DUMP_FILES_H

#include "tree_base.h"
#include <stdarg.h>
#include <stdio.h>

void CreateGraphvizDump (Node* root, const char* filename);
void CreateGraphvizHeader (FILE* dot_file);
void CreateGraphvizNodes (FILE* dot_file, Node* node);
void CreateGraphvizEdges (FILE* dot_file, Node* node);
void GenerateImage (const char* dot_filename);
void CreateHtmlDump (Node* tree, const char* func, const char* reason, ...);
void CloseHtmlFile (void);
const char* NodeTypeToString (NodeType type);
void EscapeHtml (FILE* html_file, const char* text, size_t len);
void SafePrintString (FILE* dot_file, const char* str);
void SafePrintNodeType (FILE* dot_file, NodeType type);
static void TransliterateToLatin(FILE* dot_file, const char* str);

#endif
