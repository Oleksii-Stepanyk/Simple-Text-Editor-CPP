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
	void move_cursor(TextEditor* editor);

	void _system_move_cursor(int new_row, int new_col) {
		row = new_row;
		col = new_col;
	}

	char* get_position(char** text) {
		return text[row] + col;
	}
};

class Text_Buffer {
public:
	char** text;
	int array_rows, array_cols, total_rows;
	int cursor_row, cursor_col;
	char* paste_buffer;

	Text_Buffer(char** source, int arr_rows, int arr_cols, int tot_rows, int cursor_r, int cursor_c, char* pasted_buffer)
		:array_rows(arr_rows), array_cols(arr_cols), total_rows(tot_rows), cursor_row(cursor_r), cursor_col(cursor_c) {
		text = new char* [array_rows];
		paste_buffer = new char[256];
		for (int i = 0; i < array_rows; i++) {
			text[i] = new char[array_cols];
			strcpy_s(text[i], array_cols, source[i]);
		}
		strcpy_s(paste_buffer, 256, pasted_buffer);
	}

	~Text_Buffer() {
		for (int i = 0; i < array_rows; i++) {
			delete[] text[i];
		}
		delete[] text;
		delete[] paste_buffer;
	}
};

class TextEditor {
private:
	char** text;
	int array_rows = 10, array_cols = 128, total_rows = 0;
	char* paste_buffer = new char[256];
	Text_Buffer* undo1 = nullptr;
	Text_Buffer* undo2 = nullptr;
	Text_Buffer* undo3 = nullptr;
	Text_Buffer* redo1 = nullptr;
	Text_Buffer* redo2 = nullptr;
	Text_Buffer* redo3 = nullptr;

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

	void shift_right(const int* row, const int* start_index, int shift_amount) {
		if (*start_index + shift_amount >= array_cols) {
			int new_cols = array_cols + 128;
			text = reallocate_cols(&new_cols, text);
		}
		for (int i = array_cols; i >= *start_index; i--) {
			text[*row][i + shift_amount] = text[*row][i];
		}
		text[*row][strlen(text[*row]) + 1] = '\0';
	}

	void add_undo(Cursor* cursor) {
		if (undo3) delete undo3;
		undo3 = undo2;
		undo2 = undo1;
		undo1 = new Text_Buffer(text, array_rows, array_cols, total_rows, cursor->row, cursor->col, paste_buffer);
	}

	void restore_buffer(Cursor* cursor, Text_Buffer*& buf_1, Text_Buffer*& buf_2) {
		cursor->_system_move_cursor(buf_1->cursor_row, buf_1->cursor_col);
		array_rows = buf_1->array_rows;
		array_cols = buf_1->array_cols;
		total_rows = buf_1->total_rows;

		for (int i = 0; i < array_rows; i++) {
			strcpy_s(text[i], array_cols, buf_1->text[i]);
		}
		strcpy_s(paste_buffer, 256, buf_1->paste_buffer);

		if (buf_2) delete buf_2;
		buf_2 = new Text_Buffer(buf_1->text, buf_1->array_rows, buf_1->array_cols, buf_1->total_rows, buf_1->cursor_row, buf_1->cursor_col, buf_1->paste_buffer);
		delete buf_1;
		buf_1 = nullptr;
	}

	int get_input(Cursor* cursor, string action) {
		int row = cursor->row;
		int col = cursor->col;
		int length;
		cout << "Enter the length of text to " << action << " : ";
		cin >> length;
		if (col + length >= strlen(text[row])) {
			cerr << "The length is out of range" << endl;
			return -1;
		}
		return row, col, length;
	}

public:
	TextEditor() {
		text = allocate_array();
		paste_buffer[0] = '\0';
	}

	~TextEditor() {
		deallocate_array();
		delete[] paste_buffer;

		if (undo1) delete undo1;
		if (undo2) delete undo2;
		if (undo3) delete undo3;
		if (redo1) delete redo1;
		if (redo2) delete redo2;
		if (redo3) delete redo3;
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
			<< "15: Move cursor" << endl
			<< "16: Clear console" << endl
			<< "17: Command list" << endl
			<< "0: Exit program" << endl;
	}

	void append_text(Cursor* cursor) {
		char buffer[256];
		cursor->_system_move_cursor(total_rows, (int)strlen(text[total_rows]));
		char* position = cursor->get_position(text);

		cout << "Enter text to append: " << endl;
		cin.ignore();
		cin.getline(buffer, 256);
		buffer[cin.gcount()] = '\0';

		if (strlen(text[total_rows]) + strlen(buffer) >= array_cols) {
			int new_cols = array_cols + 128;
			text = reallocate_cols(&new_cols, text);
			position = cursor->get_position(text);
			position = strstr(position, "\0");
		}
		add_undo(cursor);
		strncat_s(position, (int)strlen(buffer) + 1, (const char*)&buffer, _TRUNCATE);
		int total_cols = (int)strlen(text[total_rows]);
		cursor->_system_move_cursor(cursor->row, total_cols);
	}

	void start_newline(Cursor* cursor) {
		add_undo(cursor);
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
				cursor->_system_move_cursor(rows, cursor->col);
				start_newline(cursor);
				rows++;
				position = cursor->get_position(text);
			}
		}
		file.close();
		cout << "Text loaded successfully" << endl;
		cursor->_system_move_cursor(rows, strlen(text[rows]));
	}

	void print_text() {
		for (int i = 0; i <= total_rows; i++) {
			cout << text[i];
		}
		cout << endl;
	}

	void insert_text(Cursor* cursor) {
		int row = cursor->row;
		int col = cursor->col;
		char entered_text[64];
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		add_undo(cursor);
		shift_right(&row, &col, (int)strlen(entered_text));
		for (int i = 0; i < (int)strlen(entered_text); i++) {
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

	void delete_text(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		row, col, length = get_input(cursor, "delete");
		if (row == -1) return;
		add_undo(cursor);
		for (int i = col; i < strlen(text[row]) - length; i++) {
			text[row][i] = text[row][i + length];
		}
		text[row][strlen(text[row]) - length] = '\0';
		cout << "Text deleted successfully" << endl;
	}

	void cut_text(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		row, col, length = get_input(cursor, "cut");
		if (row == -1) return;
		add_undo(cursor);
		for (int i = col; i < col + length; i++) {
			paste_buffer[i - col] = text[row][i];
		}
		paste_buffer[length] = '\0';

		for (int i = col; i < strlen(text[row]) - length; i++) {
			text[row][i] = text[row][i + length];
		}
		cout << "Text cut successfully" << endl;
	}

	void copy_text(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		row, col, length = get_input(cursor, "copy");
		if (row == -1) return;
		add_undo(cursor);
		for (int i = col; i < col + length; i++) {
			paste_buffer[i - col] = text[row][i];
		}
		paste_buffer[length] = '\0';
		cout << "Text copied successfully" << endl;
	}

	void paste_text(Cursor* cursor) {
		if (paste_buffer[0] == '\0') {
			cerr << "The buffer is empty" << endl;
			return;
		}
		int row = cursor->row;
		int col = cursor->col;
		if (col + strlen(paste_buffer) >= array_cols - 1) {
			cerr << "The length is out of range" << endl;
			return;
		}
		add_undo(cursor);
		shift_right(&row, &col, (int)strlen(paste_buffer));
		for (int i = 0; i < (int)strlen(paste_buffer); i++) {
			text[row][col + i] = paste_buffer[i];
		}
		cout << "Text pasted successfully" << endl;
	}

	void insert_and_replace(Cursor* cursor) {
		int row = cursor->row;
		int col = cursor->col;
		char entered_text[64];
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		add_undo(cursor);
		for (int i = 0; i < (int)strlen(entered_text); i++) {
			text[row][col + i] = entered_text[i];
		}
	}

	void undo_command(Cursor* cursor) {
		if (undo1) {
			restore_buffer(cursor, undo1, redo2);
			undo1 = undo2;
			undo2 = undo3;
			undo3 = nullptr;
		}
		else {
			cerr << "Nothing to undo" << endl;
		}
	}

	void redo_command(Cursor* cursor) {
		if (redo1) {
			restore_buffer(cursor, redo1, undo1);
			redo1 = redo2;
			redo2 = undo3;
			redo3 = nullptr;
		}
		else {
			cerr << "Nothing to undo" << endl;
		}
	}

	void deallocate_array() {
		if (text != nullptr) {
			for (int i = 0; i <= array_rows; i++) {
				if (text[i] != nullptr) {
					free(text[i]);
					text[i] = nullptr;
				}
			}
			free(text);
			text = nullptr;
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
		cout << "Choose the command or enter 17 for commands list:" << endl;
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
		case 8:
			editor->delete_text(cursor);
			break;
		case 9:
			editor->undo_command(cursor);
			break;
		case 10:
			editor->redo_command(cursor);
			break;
		case 11:
			editor->cut_text(cursor);
			break;
		case 12:
			editor->copy_text(cursor);
			break;
		case 13:
			editor->paste_text(cursor);
			break;
		case 14:
			editor->insert_and_replace(cursor);
			break;
		case 15:
			cursor->move_cursor(editor);
			break;
		case 16:
			editor->clear_console();
			break;
		case 17:
			editor->print_help();
			break;
		case 0:
			delete editor;
			delete cursor;
			return 0;
		default:
			cout << "The command is not implemented" << endl;
			break;
		}
	}
}