
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"

#include <ZXing/BitMatrix.h>
#include <ZXing/BitMatrixIO.h>
#include <ZXing/CharacterSet.h>
#include <ZXing/MultiFormatWriter.h>
#include <ZXing/Version.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>  // on Ubuntu, try: #include <stb/stb_image_write.h>

using namespace ZXing;
using namespace std;

jmp_buf env;

void error_handler(HPDF_STATUS   error_no, HPDF_STATUS   detail_no, void *user_data)
{
    if ( error_no == HPDF_INVALID_FONT_NAME ) {
        printf("Unknown font name\n");
    }
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
    longjmp(env, 1);
}


vector<string> getNextLineAndSplitIntoTokens(istream& str)
{
    vector<string>   result;
    string                line;
    getline(str,line);

    stringstream          lineStream(line);
    string                cell;

    while(getline(lineStream,cell, ','))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    /*if (!lineStream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }*/
    return result;
}

vector< vector<string> > readCSVFile(string filename) {

    vector< vector<string> > allRows;

    ifstream instream( filename );

    while ( true ) {
        vector<string> row = getNextLineAndSplitIntoTokens(instream);
        if ( row.empty() )
            break;
        bool isEmpty = true;
        for ( int i = 0; i < (int)row.size(); i++ ) {
            if ( ! row[i].empty() ) {
                isEmpty = false;
                break;
            }
        }
        if ( isEmpty )
            break;
        allRows.push_back( row );
    }

    return allRows;
}

const char *font_list[] = {
    "Courier",
    "Courier-Bold",
    "Courier-Oblique",
    "Courier-BoldOblique",
    "Helvetica",
    "Helvetica-Bold",
    "Helvetica-Oblique",
    "Helvetica-BoldOblique",
    "Times-Roman",
    "Times-Bold",
    "Times-Italic",
    "Times-BoldItalic",
    NULL
};

const char* fontName = "Helvetica";
int dim = 24;
int rowsep = 10;
int colsep = 100;
int textsep = 5;
int margin = 20;
int fontSize = 9;
int reps = 1;
char qrContentColumns[32] = "0";
char commentRow1Columns[32] = "1";
char commentRow2Columns[32] = "2";
string symbolStyle = "qr";

void usage(char* progName)
{
    cout << "\nUsage: " << progName << " -i infile.csv -o outfile.pdf [options]" << endl << endl <<
        "  Options:" << endl <<
        "     -v                Print verbose output" << endl <<
        "     -x style          Symbol style, either 'qr' (QR code) or 'dm' (DataMatrix) (default = " << symbolStyle << ")" << endl <<
        "     -s size           QR code size (default = " << dim << ")" << endl <<
        "     -q columns        Column format for QR code value (default = '0')" << endl <<
        "     -1 columns        Column format for comment 1 (default = '1')" << endl <<
        "     -2 columns        Column format for comment 2 (default = '2')" << endl <<
        "     -d char           Single-character delimiter between columns (default = none)" << endl <<
        "     -f name           Font name (default = " << fontName << ")" << endl <<
        "     -h size           Font size (default = " << fontSize << ")" <<  endl <<
        "     -r distance       Separation between rows (default = " << rowsep << ")" <<  endl <<
        "     -c distance       Separation between columns (default = " << colsep << ")" <<  endl <<
        "     -t distance       Separation between QR and text (default = " << textsep << ")" <<  endl <<
        "     -m distance       Page margin (default = " << margin << ")" <<  endl <<
        "     -n count          Repetitions (default = " << reps << ")" <<  endl <<
        endl <<
        "  Note that all numeric options are integers." << endl <<
        "  Input file must be plain ASCII, not utf-8 !" << endl <<
        "  An empty line in the CSV file will be considered end of input." << endl <<
        endl <<
        "  A column format is a sequence of integers from 0-9 denoting which columns " << endl <<
        "  to concatenate into the final value. For example the format '034' would" << endl <<
        "  form a value by concatenating columns 0, 3 and 4 (positions are zero indexed)." << endl <<
        "  To insert a delimiter between the concatenations, use the -d option." << endl <<
        endl <<
        "  Valid font names:" << endl;
    int i = 0;
    while ( font_list[i] ) {
        cout << "     " << font_list[i] << endl;
        i++;
    }
    cout << endl;
}

string makeContentString(vector<string> &cols, char* format, char delimiter) {
    int i = 0;
    char c = 0;
    while ( (c = format[i++]) ) {
        int colnum = c - '0';
        if ( colnum >= (int)cols.size() ) {
            return "invalid";
        }
    }
    i = 0;
    string s = "";
    while ( (c = format[i++]) ) {
        int colnum = c - '0';
        if ( delimiter ) {
            if ( i > 1 )
                s += delimiter;
        }
        s += cols[colnum];
    }
    return s;
}

int main (int argc, char **argv)
{
    bool verbose = false;
    char* infile = 0, *outfile = 0;

    if ( argc < 2 ) {
        usage(argv[0]);
        return -1;
    }

    char delimiter = 0; // none
    sprintf(qrContentColumns, "0");
    sprintf(commentRow1Columns, "1");
    sprintf(commentRow2Columns, "2");

    int c = -1;
    while ((c = getopt(argc, argv, "i:o:vf:h:r:c:s:t:m:n:q:1:2:d:x:")) != -1) {
        switch (c) {
        case 'v':
            verbose = true;
            break;
        case 'x':
            if ( string(optarg) == "dm" )
                symbolStyle = "dm";
            break;
        case 'i':
            infile = optarg;
            break;
        case 'o':
            outfile = optarg;
            break;
        case 's':
            dim = atoi(optarg);
            break;
        case 'f':
            fontName = optarg;
            break;
        case 'h':
            fontSize = atoi(optarg);
            break;
        case 'r':
            rowsep = atoi(optarg);
            break;
        case 'c':
            colsep = atoi(optarg);
            break;
        case 't':
            textsep = atoi(optarg);
            break;
        case 'm':
            margin = atoi(optarg);
            break;
        case 'n':
            reps = atoi(optarg);
            break;
        case 'q':
            sprintf(qrContentColumns, "%s", optarg);
            break;
        case '1':
            sprintf(commentRow1Columns, "%s", optarg);
            break;
        case '2':
            sprintf(commentRow2Columns, "%s", optarg);
            break;
        case 'd':
            delimiter = optarg[0];
            break;
        }
    }

    if (optind < argc) {
        printf ("Ignoring unknown argument: ");
        while (optind < argc) {
            printf ("%s ", argv[optind++]);
        }
        printf ("\n");
    }

    if ( verbose ) {
        printf("Showing verbose output\n");
        printf("  Input file: '%s'\n", infile);
        printf("  Output file: '%s'\n", outfile);
        printf("  Symbol style: %s\n", symbolStyle.c_str());
        printf("  QR code size: %d\n", dim);
        printf("  Font name: '%s'\n", fontName);
        printf("  Font size: %d\n", fontSize);
        printf("  Row separation: %d\n", rowsep);
        printf("  Column separation: %d\n", colsep);
        printf("  Text separation: %d\n", textsep);
        printf("  Page margin: %d\n", margin);
        printf("  Repetitions: %d\n", reps);
        printf("  QR content columns: %s\n", qrContentColumns);
        printf("  Comment row 1 columns: %s\n", commentRow1Columns);
        printf("  Comment row 2 columns: %s\n", commentRow2Columns);
        printf("  Column delimiter: %c\n", delimiter);
    }

    if ( infile == 0 || outfile == 0 ) {
        usage(argv[0]);
        return -1;
    }

    ifstream f(infile);
    if ( ! f.good() ) {
        printf("File not found '%s'\n", infile);
        return -1;
    }

    vector< vector<string> > rows = readCSVFile(infile);

    if ( rows.empty() ) {
        printf("No parts found!\n");
        return -1;
    }

    if ( verbose ) {
        printf("Found %d rows\n", (int)rows.size());
        for ( int i = 0; i < (int)rows.size(); i++ ) {
            vector<string> cols = rows[i];
            for ( int k = 0; k < (int)cols.size(); k++ ) {
                printf(k > 0 ? ", " : "  ");
                printf("%s", cols[k].c_str());
            }
            printf("\n");
        }
    }

    HPDF_Doc pdf = HPDF_New (error_handler, NULL);
    if (!pdf) {
        printf ("Cannot create PDF object\n");
        return -1;
    }

    if (setjmp(env)) {
        HPDF_Free (pdf);
        return -1;
    }

    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    HPDF_Font font = HPDF_GetFont (pdf, fontName, NULL);

    HPDF_Page page = HPDF_AddPage (pdf);
    HPDF_Page_SetFontAndSize (page, font, fontSize);

    HPDF_Page_SetSize (page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Destination dst = HPDF_Page_CreateDestination (page);
    HPDF_Destination_SetXYZ (dst, 0, HPDF_Page_GetHeight (page), 1);
    HPDF_SetOpenAction(pdf, dst);

    int x = margin;
    int y = HPDF_Page_GetHeight(page) - dim - margin;

    for (int i = 0; i < (int)rows.size(); i++) {

        vector<string> cols = rows[i];

        string code     = makeContentString(cols, qrContentColumns, delimiter);
        string comment1 = makeContentString(cols, commentRow1Columns, delimiter);
        string comment2 = makeContentString(cols, commentRow2Columns, delimiter);

        if ( code == "" ) {
            printf("Ignoring empty cell in row %d !\n", i+1);
            continue;
        }

        if ( verbose ) {
            printf("Adding QRcode: x=%d, y=%d, content='%s', comment1='%s', comment2='%s'\n", x, y, code.c_str(), comment1.c_str(), comment2.c_str());
        }

        for (int rep = 0; rep < reps; rep++) {
            try {

                BarcodeFormat bcfmt = symbolStyle == "dm" ? BarcodeFormat::DataMatrix : BarcodeFormat::QRCode;

                auto writer = MultiFormatWriter( bcfmt ).setMargin(0);

                writer.setEncoding(CharacterSet::UTF8);
                BitMatrix matrix = writer.encode(code.c_str(), 256, 256);
                auto bitmap = ToMatrix<uint8_t>(matrix);

                int len = 0;
                unsigned char *png = stbi_write_png_to_mem(bitmap.data(), 256, 256, 256, 1, &len);

                HPDF_Image image = HPDF_LoadPngImageFromMem(pdf, png, len);

                HPDF_Page_DrawImage (page, image, x, y, dim, dim);

                HPDF_Page_BeginText (page);
                HPDF_Page_SetTextLeading (page, 0);
                HPDF_Page_MoveTextPos (page, x+dim+textsep, y + dim / 2 + fontSize * 0.15 );
                HPDF_Page_ShowTextNextLine (page, comment1.c_str());
                HPDF_Page_SetTextLeading (page, fontSize);
                HPDF_Page_ShowTextNextLine (page, comment2.c_str());
                HPDF_Page_EndText (page);

                y -= dim + rowsep;

                if ( y - dim < 0 ) {
                    y = HPDF_Page_GetHeight(page) - dim - margin;
                    x += colsep;
                    if ( x > HPDF_Page_GetWidth(page) - margin ) {
                        if ( verbose )
                            printf("Page break at %d\n", i);
                        page = HPDF_AddPage (pdf);
                        HPDF_Page_SetFontAndSize (page, font, fontSize);
                        x = margin;
                        y = HPDF_Page_GetHeight(page) - dim - margin;
                    }
                }

            } catch (const exception& e) {
                cerr << e.what() << endl;
                return -1;
            }
        }
    }

    HPDF_SaveToFile (pdf, outfile);
    HPDF_Free (pdf);

    return 0;
}
