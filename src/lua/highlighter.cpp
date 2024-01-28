/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include "highlighter.h"

namespace Lua {

Highlighter::Highlighter(QTextDocument *parent)
     : QSyntaxHighlighter(parent)
{
	/**
	 * This is the syntax highlighter copied from the example in the qt4 docs,
	 * and modified look for lua style text.
	 *
	 * Each rule is applied in order to the text in a given block.  The last
	 * rule to match wins.
	 */
	HighlightingRule rule;

	constantFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("[-+]?(?:(?:\\d+\\.\\d+)|(?:\\.\\d+)|(?:\\d+\\.?))");
	rule.format = constantFormat;
	highlightingRules.append(rule);

	classFormat.setForeground(Qt::darkMagenta);
	rule.pattern = QRegExp("\\b[A-Za-z][A-Za-z0-9_]*\\b");
	rule.format = classFormat;
	highlightingRules.append(rule);

	functionFormat.setForeground(Qt::blue);
	rule.pattern = QRegExp("\\b[A-Za-z0-9_]+ *(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	QStringList keywordPatterns;
	keywordPatterns
			<< "\\bdo\\b" << "\\bend\\b" << "\\bfor\\b" << "\\bwhile\\b"
			<< "\\bif\\b" << "\\bthen\\b" << "\\belse\\b" << "\\belseif\\b"
			<< "\\brepeat\\b" << "\\buntil\\b" << "\\bfunction\\b" << "\\blocal\\b"
			<< "\\breturn\\b" << "\\bin\\b" << "\\bbreak\\b" << "\\bnot\\b"
			<< "\\bnil\\b" << "\\band\\b" << "\\bor\\b" << "\\btrue\\b"
			<< "\\bfalse\\b";
	foreach (QString pattern, keywordPatterns)
	{
		rule.pattern = QRegExp(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	quotationFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("\"[^\"]*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegExp("'[^']*'");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	singleLineCommentFormat.setForeground(Qt::gray);
	singleLineCommentFormat.setFontItalic(true);
	rule.pattern = QRegExp("--[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	multiLineCommentFormat.setForeground(Qt::gray);
	multiLineCommentFormat.setFontItalic(true);

	multiLineStringExpression  = QRegExp("\\[(=*)\\[");
	multiLineCommentExpression = QRegExp("--[^\n]*\\[=*\\[");
}


void Highlighter::highlightBlock(const QString &text)
{
	setCurrentBlockState(0);

	int level = 0;
	int iscomment = 0;
	int startIndex = 0;
	int previousState = previousBlockState();

	if (previousState < 1)
	{
		// Apply these rules if not in a multi-line comment block
		foreach (HighlightingRule rule, highlightingRules)
		{
			QRegExp expression(rule.pattern);
			int index = text.indexOf(expression);
			while (index >= 0)
			{
				int length = expression.matchedLength();
				setFormat(index, length, rule.format);
				index = text.indexOf(expression, index + length);
			}
		}

		// Look for a mutli-line string in this block
		startIndex = text.indexOf(multiLineStringExpression);
	}
	else
	{
		level     = (previousState & 0x0f) - 1;
		iscomment =  previousState & 0x10;
	}

	//
	// Determine if the current block is in a multi-line comment/string.
	//
	// The current comment level is saved in the current block state.  The
	// comment level is equal to the number of '=' characters in a long string
	// token (eg. "[==[" has level 2).
	//
	// A block state value greater than zero indicates a previous or current
	// comment block.  The comment level is one less than the block state
	// value.  A flag in bit four indicates if the block is a multi-line
	// comment or a quoted string.
	//
	while (startIndex >= 0)
	{
		if (multiLineStringExpression.matchedLength() > 0)
		{
			level = multiLineStringExpression.cap(1).length();
			int idx = text.indexOf(multiLineCommentExpression);
			if (idx >= 0)
			{
				startIndex = idx;
				iscomment  = 0x10;
			}
		}

		QRegExp endExpression(QString("\\]={%1}\\]").arg(level));
		int endIndex = text.indexOf(endExpression, startIndex);
		int commentLength;
		if (endIndex == -1)
		{
			setCurrentBlockState(1 + level + iscomment);
			commentLength = text.length() - startIndex;
		}
		else
		{
			commentLength = endIndex - startIndex
				+ endExpression.matchedLength();
		}
		setFormat(startIndex, commentLength,
			iscomment ? multiLineCommentFormat : quotationFormat);
		startIndex = text.indexOf(multiLineStringExpression,
			startIndex + commentLength);
	}
}

};
