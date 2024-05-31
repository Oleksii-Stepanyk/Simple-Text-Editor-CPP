#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

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
			<< "15: Clear console" << endl
			<< "0: Exit program" << endl;
	}

	char* append_text(char* last_char) {
		char buffer[256];
		cout << "Enter text to append: " << endl;
		cin.ignore();
		cin.getline(buffer, 256);
		buffer[cin.gcount()] = '\0';
		int a = (int)strlen(buffer);
		if (strlen(text[total_rows]) + strlen(buffer) >= array_cols) {
			int new_cols = array_cols + 128;
			text = reallocate_cols(&new_cols, text);
			last_char = strstr(text[total_rows], "\0");
			strcat_s(last_char, strlen(buffer), (const char*)&buffer);
		}
		else {
			strcat_s(last_char, strlen(buffer) + 1, (const char*)&buffer);
		}
		return last_char + strlen(buffer);
	}

	char* start_newline(char* last_char) {
		*last_char = '\n';
		last_char++;
		*last_char = '\0';
		if (total_rows + 1 >= array_rows) {
			int new_rows = array_rows + 10;
			text = reallocate_rows(&new_rows, text);
		}
		total_rows++;
		*text[total_rows] = '\0';
		return text[total_rows];
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

	char* load_file() // BIG PROBLEM 
	{
		char file_name[32];
		char buffer[256];
		const char* p_b = (char*)&buffer;
		int rows = 0;
		cout << "Enter the file name for loading: ";
		cin >> file_name;
		ifstream file(file_name);
		if (file.fail()) {
			printf("Error opening file\n");
			return NULL;
		}
		while (file.getline(buffer, 256)) {
			if (strlen(text[rows]) + strlen(buffer) >= array_cols) {
				int new_cols = array_cols + 128;
				text = reallocate_cols(&new_cols, text);
			}
			strncat_s(text[rows], array_cols, buffer, _TRUNCATE);

			if (!file.eof() && (file.peek() == '\n' || file.peek() == '\r')) {
				if (strlen(text[rows]) + 1 >= array_cols) {
					int new_cols = array_cols + 128;
					text = reallocate_cols(&new_cols, text);
				}
				strncat_s(text[rows], array_cols, "\n", _TRUNCATE);
				file.get();
			}

			if (strchr(text[rows], '\n') != NULL) {
				if (total_rows + 1 >= array_rows) {
					int new_rows = array_rows + 10;
					text = reallocate_rows(&new_rows, text);
				}
				text[rows + 1] = start_newline(text[rows] + strlen(text[rows]));
				text[rows][strlen(text[rows]) - 1] = '\0';
				rows++;
			}
		}
		file.close();
		return text[rows] + strlen(text[rows]);
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

	void insert_text() {
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
				position[i] = start_newline(text[total_rows] + strlen(text[total_rows]));
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

int main() {
	TextEditor* editor = new TextEditor();
	char* last_char = *editor->get_text();
	*last_char = '\0';
	while (true) {
		int input;
		cout << "Choose the command or enter 9 for commands list:" << endl;
		cin >> input;
		switch (input) {
		case 1:
			last_char = editor->append_text(last_char);
			break;
		case 2:
			last_char = editor->start_newline(last_char);
			break;
		case 3:
			editor->save_file();
			break;
		case 4:
			last_char = editor->load_file();
			break;
		case 5:
			editor->print_text();
			break;
		case 6:
			editor->insert_text();
			break;
		case 7:
			editor->search_text();
			break;
		case 8:
			editor->clear_console();
			break;
		case 9:
			editor->print_help();
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