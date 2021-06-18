// Tetris.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
// Done following javidx9 tutorial for Quick and Simple C++ Tetris!

#include <iostream>
#include <cmath>
#include <Windows.h>
#include <thread>
#include <vector>
using namespace std;

const int EMPTY_SPACE = 0;
const int DEGREES_0 = 0;
const int DEGREES_90 = 1;
const int DEGREES_180 = 2;
const int DEGREES_270 = 3;
const int MATRIX_WIDTH = 4;         // Width of the array that stores a piece. In this case 4x4.
const int BORDER = 9;
const int ASCII_OFFSET = 65;        // To skip to the letter A
const int NUM_OF_USER_KEYS = 4;
const int NUM_OF_PIECES = 7;
const int SPEED_INCREASE_THRESHOLD = 10; // Number of pieces spawned between every increase in speed


std::wstring pieces[NUM_OF_PIECES];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

/* The follow values must match your CMD Screen buffer size >>>> Right click on prompt > Properties > Layout */
int nScreenWidth = 120;      // Console Screen Size X in columns
int nScreenHeight = 300;     // Console Screen Size X in columns

void SetPieces();
int Rotate(int piece_x, int piece_y, int rotation);
bool DoesPieceFit(int nPiece, int nRotation, int nPosX, int nPosY);

int main()
{
    int nSpawnOffset = 2;

    SetPieces();

    pField = new unsigned char[nFieldWidth * nFieldHeight];
    for (int x = 0; x < nFieldWidth; x++) {
        for (int y = 0; y < nFieldHeight; y++)
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? BORDER : EMPTY_SPACE;
    }

    wchar_t *screen = new wchar_t[nScreenHeight * nScreenWidth];
    for (int i = 0; i < nScreenHeight*nScreenWidth; i++)
    {
        screen[i] = ' ';
    }
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    bool bGameOver = false;

    int nCurrentPiece = 0;
    int nCurrentPieceRotation = 0;
    int nCurrentPieceX = nFieldWidth / 2;
    int nCurrentPieceY = 0;
    int nSpeed = 20;
    int nGameTickCounter = 0;
    int nPieceCount = 0;
    int nScore = 0;

    bool bKey[NUM_OF_USER_KEYS];
    bool bRotateHold = false;
    bool bForceDown = false;

    vector<int> vLines;

    while (!bGameOver)
    {
        // Timing
        this_thread::sleep_for(50ms);   // Game tick
        nGameTickCounter++;
        bForceDown = (nGameTickCounter == nSpeed);

        // Input
        for (int k = 0; k < NUM_OF_USER_KEYS; k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;      // String: hexadecimal values for the LRD Arrows + Z keys. If pressed returns true.

        // Game logic
        if (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentPieceRotation, nCurrentPieceX + 1, nCurrentPieceY)) {
                nCurrentPieceX++;
        }

        if (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentPieceRotation, nCurrentPieceX - 1, nCurrentPieceY)) {
                nCurrentPieceX--;
        }

        if (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentPieceRotation, nCurrentPieceX, nCurrentPieceY + 1)) {
                nCurrentPieceY++;
        }

        if (bKey[3] && DoesPieceFit(nCurrentPiece, nCurrentPieceRotation + 1, nCurrentPieceX, nCurrentPieceY) && !bRotateHold) {
            nCurrentPieceRotation++;
            bRotateHold = true;
        }
        else if(!bKey[3])
            bRotateHold = false;

        if (bForceDown)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentPieceRotation, nCurrentPieceX, nCurrentPieceY + 1))
                nCurrentPieceY++;
            else {
                // Lock into field
                for (int x = 0; x < MATRIX_WIDTH; x++)
                    for (int y = 0; y < MATRIX_WIDTH; y++)
                        if (pieces[nCurrentPiece][Rotate(x, y, nCurrentPieceRotation)] == L'X')
                            pField[(nCurrentPieceY + y) * nFieldWidth + (nCurrentPieceX + x)] = nCurrentPiece + 1;

                nPieceCount++;
                if (nPieceCount % SPEED_INCREASE_THRESHOLD == 0)
                    if (nSpeed >= 10)
                        nSpeed--;

                // Check if there are full lines
                for (int y = 0; y < MATRIX_WIDTH; y++)
                    if (nCurrentPieceY + y < nFieldHeight - 1)
                    {
                        bool bLine = true;
                        for (int x = 1; x < nFieldWidth - 1; x++)
                            bLine &= (pField[(nCurrentPieceY + y) * nFieldWidth + x]) != 0;

                        if (bLine)
                        {
                            // Visually warn player that formed a line by changing it to "="
                            for (int x = 1; x < nFieldWidth - 1; x++)
                                pField[(nCurrentPieceY + y) * nFieldWidth + x] = 8;
                            vLines.push_back(nCurrentPieceY + y);
                        }
                    }

                nScore += 25;
                if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;      // Exponential score win for removing more than one line in one go

                // Choose next piece
                nCurrentPieceX = nFieldWidth / 2;
                nCurrentPieceY = 0;
                nCurrentPieceRotation = 0;
                nCurrentPiece = rand() % NUM_OF_PIECES;

                // If it does not fit its game over
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentPieceRotation, nCurrentPieceX, nCurrentPieceY);
            }

            nGameTickCounter = 0;
        }

        // Draw field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + nSpawnOffset) * nScreenWidth + (x + nSpawnOffset)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

        // Draw Current Piece
        for (int x = 0; x < MATRIX_WIDTH; x++)
            for (int y = 0; y < MATRIX_WIDTH; y++)
                if (pieces[nCurrentPiece][Rotate(x, y, nCurrentPieceRotation)] == L'X')
                    screen[(nCurrentPieceY + y + nSpawnOffset) * nScreenWidth + (nCurrentPieceX + x + nSpawnOffset)] = nCurrentPiece + ASCII_OFFSET;

        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        // Animate Line Completion
        if (!vLines.empty())
        {
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v : vLines)
                for (int x = 1; x < nFieldWidth - 1; x++)
                {
                    for (int y = v; y > 0; y--)
                        pField[y * nFieldWidth + x] = pField[(y - 1) * nFieldWidth + x];
                    pField[x] = 0;
                }

            vLines.clear();
        }

        // Display frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    CloseHandle(hConsole);
    cout << "Game Over - Final Score: " << nScore << endl;

    return 0;
}

void SetPieces() {
    pieces[0].append(L".X..");
    pieces[0].append(L".X..");
    pieces[0].append(L".X..");
    pieces[0].append(L".X..");

    pieces[1].append(L"....");
    pieces[1].append(L".XX.");
    pieces[1].append(L".XX.");
    pieces[1].append(L"....");

    pieces[2].append(L".X..");
    pieces[2].append(L".XX.");
    pieces[2].append(L"..X.");
    pieces[2].append(L"....");

    pieces[3].append(L"..X.");
    pieces[3].append(L".XX.");
    pieces[3].append(L".X..");
    pieces[3].append(L"....");

    pieces[4].append(L"..X.");
    pieces[4].append(L".XX.");
    pieces[4].append(L"..X.");
    pieces[4].append(L"....");

    pieces[5].append(L"....");
    pieces[5].append(L".XX.");
    pieces[5].append(L"..X.");
    pieces[5].append(L"..X.");

    pieces[6].append(L".XX.");
    pieces[6].append(L".X..");
    pieces[6].append(L".X..");
    pieces[6].append(L"....");
}

int Rotate(int piece_x, int piece_y, int rotation) {
    switch (rotation % 4)
    {
    case DEGREES_0:
        return piece_y * MATRIX_WIDTH + piece_x;                                            // i = wy + x
        break;
    case DEGREES_90:
        return MATRIX_WIDTH * (MATRIX_WIDTH - 1) + piece_y - MATRIX_WIDTH * piece_x;        // i = w * (w - 1) + y - w * x
        break;
    case DEGREES_180:
        return pow(MATRIX_WIDTH, 2) - (MATRIX_WIDTH * piece_y) - piece_x;                     // i = w ^ 2 - 1 - w * y - x
        break;
    case DEGREES_270:
        return MATRIX_WIDTH - 1 - piece_y + MATRIX_WIDTH * piece_x;                         // i = w - 1 - y + w * x
        break;
    }
}

bool DoesPieceFit(int nPiece, int nRotation, int nPosX, int nPosY) {
    for(int x = 0; x < MATRIX_WIDTH; x++)
        for (int y = 0; y < MATRIX_WIDTH; y++)
        {
            // Get Index into piece
            int pieceIndex = Rotate(x, y, nRotation);

            // Get index into field
            int fieldIndex = (nPosY + y) * nFieldWidth + (nPosX + x);

            if (nPosX + x >= 0 && nPosX + x < nFieldWidth){
                if (nPosY + y >= 0 && nPosY + y < nFieldHeight) {
                    if (pieces[nPiece][pieceIndex] == L'X' && pField[fieldIndex] != 0)
                        return false;   // colision
                }
            }
        }

    return true;
}
