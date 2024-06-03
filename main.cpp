#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
using namespace std;

class TextEditor;

class Cursor {
public:
	int row, col;

	Cursor() {
		row = 0;
		col = 0;
	}

	void _system_move_cursor(int new_row, int new_col) {
		row = new_row;
		col = new_col;
	}

	void move_cursor(TextEditor* editor);

	char* get_position(char** text) {
		return text[row] + col;
	}
};

class TextEditor {
private:
	char** text;
	int array_rows = 10, array_cols = 128, total_rows = 0;

	char** allocate_array() {
		char** array = (char**)malloc(array_rows * sizeof(char*));
		if (array == NULL) {
			cerr << "Failed to allocate memory";
			return NULL;
		}
		for (int i = 0; i < array_rows; i++) {
			array[i] = (char*)malloc(array_cols * sizeof(char));
			if (array[i] == NULL) {
				cerr << "Failed to allocate memory";
				return NULL;
			}
			array[i][0] = '\0';
		}
		return array;
	}

	char** reallocate_rows(const int* new_rows, char** array) {
		array = (char**)realloc(array, *new_rows * sizeof(char*));
		if (array == NULL) {
			cerr << "Failed to allocate memory";
			return NULL;
		}
		for (int i = array_rows; i < *new_rows; i++) {
			array[i] = (char*)malloc(array_cols * sizeof(char));
			if (array[i] == NULL) {
				cerr << "Failed to allocate memory";
				return NULL;
			}
		}
		array_rows = *new_rows;
		return array;
	}

	char** reallocate_cols(const int* new_cols, char** array) {
		for (int i = 0; i < array_rows; i++) {
			array[i] = (char*)realloc(array[i], *new_cols * sizeof(char));
			if (array[i] == NULL) {
				cerr << "Failed to allocate memory";
				return NULL;
			}
		}
		array_cols = *new_cols;
		return array;
	}

	void shift_right(const int* row, const int* start_index, const int* shift_amount) {
		if (*start_index + *shift_amount >= array_cols) {
			int new_cols = array_cols + 128;
			text = reallocate_cols(&new_cols, text);
		}
		for (int i = array_cols; i >= *start_index; i--) {
			text[*row][i + *shift_amount] = text[*row][i];
		}
		text[*row][strlen(text[*row]) + 1] = '\0';
	}

public:
	TextEditor() {
		text = allocate_array();
	}

	char** get_text() {
		return text;
	}

	int get_rows() {
		return total_rows;
	}

	void print_help() {
		cout << "Commands: " << endl
			<< "1: Append text symbols to the end" << endl
			<< "2: Start the new line" << endl
			<< "3: Save text into file" << endl
			<< "4: Load text from file" << endl
			<< "5: Print the current text" << endl
			<< "6: Insert text by line and index" << endl
			<< "7: Search text placement" << endl
			<< "8: Delete text" << endl
			<< "9: Undo command" << endl
			<< "10: Redo command" << endl
			<< "11: Cut text" << endl
			<< "12: Copy text" << endl
			<< "13: Paste text" << endl
			<< "14: Insert text with replacement" << endl
			<< "15: Command list" << endl
			<< "16: Move cursor" << endl
			<< "17: Clear console" << endl
			<< "0: Exit program" << endl;
	}

	void append_text(Cursor* cursor) {
		char buffer[256];
		char* position = cursor->get_position(text);
		cout << "Enter text to append: " << endl;
		cin.ignore();
		cin.getline(buffer, 256);
		buffer[cin.gcount()] = '\0';
		if (strlen(position) + strlen(buffer) >= array_cols) {
			int new_cols = array_cols + 128;
			text = reallocate_cols(&new_cols, text);
			position = cursor->get_position(text);
			position = strstr(position, "\0");
		}
		strncat_s(position, (int)strlen(buffer) + 1, (const char*)&buffer, _TRUNCATE);
		int total_cols = (int)strlen(text[total_rows]);
		cursor->_system_move_cursor(cursor->row, total_cols);
	}

	void start_newline(Cursor* cursor) {
		char* position = cursor->get_position(text);
		*position = '\n';
		position++;
		*position = '\0';
		if (cursor->row + 1 >= array_rows) {
			int new_rows = array_rows + 10;
			text = reallocate_rows(&new_rows, text);
		}
		total_rows++;
		cursor->_system_move_cursor(cursor->row + 1, 0);
		*cursor->get_position(text) = '\0';
	}

	void save_file() {
		char file_name[32];
		cout << "Enter the file name for saving: ";
		cin >> file_name;
		ofstream file(file_name);
		if (!file.fail()) {
			for (int i = 0; i <= total_rows; i++) {
				file << text[i];
			}
			file.close();
			cout << "Text saved successfully" << endl;
		}
	}

	void load_file(Cursor* cursor)
	{
		char file_name[32];
		char ch;
		int rows = 0;
		cursor->_system_move_cursor(0, 0);
		char* position = cursor->get_position(text);
		cout << "Enter the file name for loading: ";
		cin >> file_name;
		ifstream file(file_name);
		if (!file.is_open()) {
			cerr << "Error opening file" << endl;
			return;
		}
		while (file.get(ch)) {
			if (strlen(text[rows]) + 1 >= array_cols) {
				int new_cols = array_cols + 128;
				text = reallocate_cols(&new_cols, text);
				position = cursor->get_position(text);
			}
			if (ch != '\n') {
				*position = ch;
				position++;
				*position = '\0';
				cursor->_system_move_cursor(rows, cursor->col + 1);
			}
			else if (ch == '\n') {
				if (total_rows + 1 >= array_rows) {
					int new_rows = array_rows + 10;
					text = reallocate_rows(&new_rows, text);
					position = cursor->get_position(text);
				}
				cursor->_system_move_cursor(rows, cursor->col + 1);
				start_newline(cursor);
				rows++;
				position = cursor->get_position(text);
			}
		}
		file.close();
		cursor->_system_move_cursor(rows, strlen(text[rows]));
	}

	void print_text() {
		for (int i = 0; i <= total_rows; i++) {
			cout << text[i];
		}
		cout << endl;
	}

	void deallocate_array() {
		for (int i = 0; i <= array_rows; i++) {
			free(text[i]);
		}
		free(text);
	}

	void insert_text(Cursor* cursor) {
		int row, col;
		char entered_text[64];
		cout << "Enter the row and column to insert text: ";
		cin >> row >> col;
		if (row > array_rows) {
			int new_rows = array_rows + 10;
			text = reallocate_rows(&new_rows, text);
		}
		char** position = text;
		if (row > total_rows) {
			for (int i = total_rows + 1; i <= row; i++) {
				start_newline(cursor);
			}
		}
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		int shift_amount = (int)strlen(entered_text);
		shift_right(&row, &col, &shift_amount);
		for (int i = 0; i < shift_amount; i++) {
			text[row][col + i] = entered_text[i];
		}
	}

	void search_text() {
		char search_text[64];
		char* position;
		bool found = false;
		int index;
		cout << "Enter text you want to find:";
		cin.ignore();
		cin.getline(search_text, 64);
		search_text[cin.gcount()] = '\0';
		cout << "Text found in: ";
		for (int i = 0; i <= total_rows; ++i) {
			position = text[i];
			while ((position = strstr(position, search_text)) != NULL) {
				index = (int)(position - text[i]);
				cout << i << " " << index << "; ";
				position += strlen(search_text);
				found = true;
			}
		}
		cout << endl;
		if (!found) {
			cout << "Text not found" << endl;
		}
	}

	void clear_console() {
#ifdef _WIN64
		system("cls");
#else
		system("clear");
#endif
	}
};

void Cursor::move_cursor(TextEditor* editor) {
	int new_row, new_col;
	int total_rows = editor->get_rows();
	char** text = editor->get_text();
	cout << "Enter the row and column to move cursor: ";
	cin >> new_row >> new_col;
	if (new_row > total_rows) {
		cout << "Row is out of range" << endl;
		return;
	}
	if (new_col > strlen(text[new_row])) {
		cout << "Column is out of range" << endl;
		return;
	}
	Cursor::row = new_row;
	Cursor::col = new_col;
}

int main() {
	TextEditor* editor = new TextEditor();
	Cursor* cursor = new Cursor();
	while (true) {
		int input;
		cout << "Choose the command or enter 15 for commands list:" << endl;
		cin >> input;
		switch (input) {
		case 1:
			editor->append_text(cursor);
			break;
		case 2:
			editor->start_newline(cursor);
			break;
		case 3:
			editor->save_file();
			break;
		case 4:
			editor->load_file(cursor);
			break;
		case 5:
			editor->print_text();
			break;
		case 6:
			editor->insert_text(cursor);
			break;
		case 7:
			editor->search_text();
			break;
		case 15:
			editor->print_help();
			break;
		case 16:
			cursor->move_cursor(editor);
			break;
		case 17:
			editor->clear_console();
			break;
		case 0:
			editor->deallocate_array();
			return 0;
		default:
			cout << "The command is not implemented" << endl;
			break;
		}
	}
}