/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
 *   dbitsef@zipcon.net                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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

}

void Highlighter::highlightBlock(const QString &text)
{
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
}


}
