/* Copyright(C) 2016 Björn Stresing, Denis Manthey, Wolfgang Ruppel
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 */
#include "MetadataExtractorCommon.h"
#include <QTextCursor>
#include <QTextTable>
#include <QFontMetrics>
#include <QAbstractTextDocumentLayout>


Metadata::Metadata(eEssenceType type /*= Unknown_Type*/) :
type(type),
editRate(), // frame rate for video and sampling rate for audio milliseconds for timed text
aspectRatio(),
storedWidth(0),
storedHeight(0),
displayWidth(0),
displayHeight(0),
colorEncoding(Metadata::Unknown_Color_Encoding),
horizontalSubsampling(0),
componentDepth(0),
duration(),
audioChannelCount(0),
audioQuantization(0),
soundfieldGroup(),
fileName(),
filePath(),
profile(),
infoEditRate()
{
}

QString Metadata::GetAsString() {

	QString ret(QObject::tr("Essence Type: "));
	switch(type) {
		case Metadata::Jpeg2000:
			if(type == Metadata::Jpeg2000)						ret.append(QObject::tr("%1").arg("Jpeg2000\n"));
			if(duration.IsValid() && editRate.IsValid())	ret.append(QObject::tr("Duration: %1\n").arg(duration.GetAsString(editRate)));
			if(editRate.IsValid() == true)								ret.append(QObject::tr("Frame Rate: %1\n").arg(editRate.GetQuotient()));
			if(storedHeight != 0 || storedWidth != 0)			ret.append(QObject::tr("Stored Resolution: %1 x %2\n").arg(storedWidth).arg(storedHeight));
			if(displayHeight != 0 || displayWidth != 0)		ret.append(QObject::tr("Displayed Resolution: %1 x %2\n").arg(displayWidth).arg(displayHeight));
			if(aspectRatio != ASDCP::Rational())					ret.append(QObject::tr("Aspect Ratio: %1 (%2:%3)\n").arg(aspectRatio.Quotient()).arg(aspectRatio.Numerator).arg(aspectRatio.Denominator));
			if(horizontalSubsampling != 0 && colorEncoding != Unknown_Color_Encoding) {
				if(colorEncoding == Metadata::RGBA)					ret.append(QObject::tr("Color Mode: %1").arg("RGB"));
				else if(colorEncoding == Metadata::CDCI)		ret.append(QObject::tr("Color Mode: %1").arg("YUV"));
				ret.append(QObject::tr("(%1:%2:%3)\n").arg(4).arg(4 / horizontalSubsampling).arg(4 / horizontalSubsampling));
			}
			if(componentDepth != 0)												ret.append(QObject::tr("Color Depth: %1 bit\n").arg(componentDepth));
			break;
		case Metadata::Pcm:
			ret.append(QObject::tr("%1").arg("Pcm\n"));
			if(duration.IsValid() == false && editRate.IsValid())	ret.append(QObject::tr("Duration: %1\n").arg(duration.GetAsString(editRate)));
			if(editRate.IsValid() == true)								ret.append(QObject::tr("Sampling Rate: %1 Hz\n").arg(editRate.GetQuotient()));
			if(audioQuantization != 0)										ret.append(QObject::tr("Bit Depth: %1 bit\n").arg(audioQuantization));
			if(audioChannelCount != 0)										ret.append(QObject::tr("Channels: %1\n").arg(audioChannelCount));
			ret.append(QObject::tr("Channel Configuration: %1\n").arg(soundfieldGroup.GetName()));
			break;
		case Metadata::TimedText:
			ret.append(QObject::tr("%1").arg("Timed Text\n"));
			ret.append(QObject::tr("Duration: %1\n").arg(duration.GetAsString(editRate)));
			if (infoEditRate.IsValid())
				ret.append(QObject::tr("Duration: %1\n").arg(Duration(duration.GetCount() / 1000.*infoEditRate.GetQuotient()).GetAsString(infoEditRate)));
			else
				ret.append(QObject::tr("Duration: %1s\n").arg(duration.GetCount()/1000.));

			if (infoEditRate.IsValid())
				ret.append(QObject::tr("Edit Rate: %1 fps\n").arg(infoEditRate.GetQuotient()));
			else
				ret.append(QObject::tr("Edit Rate: not set\n"));
			ret.append(QObject::tr("Profile: %1\n").arg(profile.remove(0, 40)));
			break;
	default:
			ret = QObject::tr("Unknown\n");
			break;
	}
	ret.chop(1); // remove last \n
	return ret;
}

void Metadata::GetAsTextDocument(QTextDocument &rDoc) {

	rDoc.setDocumentMargin(2.5);
	QTextCursor cursor(&rDoc);

	QTextTableFormat tableFormat;
	tableFormat.setBorder(.5);
	tableFormat.setCellSpacing(0);
	tableFormat.setCellPadding(2);
	tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
	qreal column_width = (rDoc.size().width() - rDoc.documentMargin() * 2. - tableFormat.border() * 6. - tableFormat.cellSpacing() * 3. /*documentation image wrong?*/) / 2.;
	QVector<QTextLength> columnConstraints;
	columnConstraints << QTextLength(QTextLength::FixedLength, column_width);
	columnConstraints << QTextLength(QTextLength::FixedLength, column_width);
	tableFormat.setColumnWidthConstraints(columnConstraints);

	int column_text_width = column_width - tableFormat.cellPadding() * 2.;
	switch(rDoc.defaultTextOption().wrapMode()) {
		case QTextOption::WordWrap:
		case QTextOption::WrapAnywhere:
		case QTextOption::WrapAtWordBoundaryOrAnywhere:
			column_text_width = 2000;
			break;
	}

	QFontMetrics font_metrics(rDoc.defaultFont());

	if(type == Metadata::Jpeg2000) {

		QTextTable *table = cursor.insertTable(5, 2, tableFormat);
		switch(type) {
			case Metadata::Jpeg2000:																				table->cellAt(0, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Essence Type: %1").arg("Jpeg2000"), Qt::ElideRight, column_text_width)); break;
			default:																												table->cellAt(0, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Essence Type: Unknown"), Qt::ElideRight, column_text_width)); break;
		}
		if(duration.IsValid() && editRate.IsValid())											table->cellAt(0, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Duration: %1").arg(duration.GetAsString(editRate)), Qt::ElideRight, column_text_width));
		else																															table->cellAt(0, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Duration: Unknown"), Qt::ElideRight, column_text_width));
		if(editRate.IsValid())																						table->cellAt(1, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Frame Rate: %1").arg(editRate.GetQuotient()), Qt::ElideRight, column_text_width));
		else																															table->cellAt(1, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Frame Rate: Unknown"), Qt::ElideRight, column_text_width));
		if(storedHeight != 0 || storedWidth != 0)													table->cellAt(1, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Stored Resolution: %1 x %2").arg(storedWidth).arg(storedHeight), Qt::ElideRight, column_text_width));
		else																															table->cellAt(1, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Stored Resolution: Unknown"), Qt::ElideRight, column_text_width));
		if(aspectRatio != ASDCP::Rational())															table->cellAt(2, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Aspect Ratio: %1 (%2:%3)").arg(aspectRatio.Quotient()).arg(aspectRatio.Numerator).arg(aspectRatio.Denominator), Qt::ElideRight, column_text_width));
		else																															table->cellAt(2, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Aspect Ratio: Unknown"), Qt::ElideRight, column_text_width));
		if(displayHeight != 0 || displayWidth != 0)												table->cellAt(2, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Displayed Resolution: %1 x %2").arg(displayWidth).arg(displayHeight), Qt::ElideRight, column_text_width));
		else																															table->cellAt(2, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Displayed Resolution: Unknown"), Qt::ElideRight, column_text_width));
		if(horizontalSubsampling != 0 && colorEncoding != Metadata::Unknown_Color_Encoding) {
			if(colorEncoding == Metadata::RGBA)															table->cellAt(3, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Color Mode: %1(%2:%3:%4)").arg("RGB").arg(4).arg(4 / horizontalSubsampling).arg(4 / horizontalSubsampling), Qt::ElideRight, column_text_width));
			else if(colorEncoding == Metadata::CDCI)												table->cellAt(3, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Color Mode: %1(%2:%3:%4)").arg("YUV").arg(4).arg(4 / horizontalSubsampling).arg(4 / horizontalSubsampling), Qt::ElideRight, column_text_width));
		}
		else																															table->cellAt(3, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Color Mode: Unknown"), Qt::ElideRight, column_text_width));
		if(componentDepth != 0)																						table->cellAt(3, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Color Depth: %1 bit").arg(componentDepth), Qt::ElideRight, column_text_width));
		else																															table->cellAt(3, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Color Depth: Unknown"), Qt::ElideRight, column_text_width));
	}
	else if(type == Metadata::Pcm) {

		QTextTable *table = cursor.insertTable(4, 2, tableFormat);
		switch(type) {
			case Metadata::Pcm:																							table->cellAt(0, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Essence Type: %1").arg("Pcm"), Qt::ElideRight, column_text_width)); break;
			default:																												table->cellAt(0, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Essence Type: Unknown"), Qt::ElideRight, column_text_width)); break;
		}
		if(duration.IsValid() && editRate.IsValid())											table->cellAt(0, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Duration: %1").arg(duration.GetAsString(editRate)), Qt::ElideRight, column_text_width));
		else																															table->cellAt(0, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Duration: Unknown"), Qt::ElideRight, column_text_width));
		if(editRate.IsValid())																						table->cellAt(1, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Sampling Rate: %1 Hz").arg(editRate.GetQuotient()), Qt::ElideRight, column_text_width));
		else																															table->cellAt(1, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Sampling Rate: Unknown"), Qt::ElideRight, column_text_width));
		if(audioQuantization != 0)																				table->cellAt(1, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Bit Depth: %1 bit").arg(audioQuantization), Qt::ElideRight, column_text_width));
		else																															table->cellAt(1, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Bit Depth: Unknown"), Qt::ElideRight, column_text_width));
		if(audioChannelCount != 0)																				table->cellAt(2, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Channels: %1").arg(audioChannelCount), Qt::ElideRight, column_text_width));
		else																															table->cellAt(2, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Channels: Unknown"), Qt::ElideRight, column_text_width));
		table->cellAt(2, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Channel Configuration: %1").arg(soundfieldGroup.GetName()), Qt::ElideRight, column_text_width));
	}
	else if(type == Metadata::TimedText) {

		QTextTable *table = cursor.insertTable(4, 2, tableFormat);
		if(is_ttml_file(filePath))
			table->cellAt(0, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Source File Name: %1").arg(fileName), Qt::ElideRight, column_text_width));
		table->cellAt(0, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Essence Type: %1").arg("Timed Text"), Qt::ElideRight, column_text_width));
		table->cellAt(1, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Profile: %1").arg(profile.remove(0, 40)), Qt::ElideRight, column_text_width));

		if (infoEditRate.IsValid())
			table->cellAt(1, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Duration: %1").arg(Duration(duration.GetCount() / 1000.*infoEditRate.GetQuotient()).GetAsString(infoEditRate)), Qt::ElideRight, column_text_width));
		else
			table->cellAt(1, 1).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Duration: %1s").arg(duration.GetCount()/1000.), Qt::ElideRight, column_text_width));

		if (infoEditRate.IsValid())
			table->cellAt(2, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Edit Rate: %1 fps").arg(infoEditRate.GetQuotient()), Qt::ElideRight, column_text_width));
		else
			table->cellAt(2, 0).firstCursorPosition().insertText(font_metrics.elidedText(QObject::tr("Edit Rate: not set"), Qt::ElideRight, column_text_width));
	}
}
