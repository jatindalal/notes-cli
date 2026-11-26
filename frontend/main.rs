use std::ffi::{c_int, c_char, c_void, CString, CStr};
use std::vec::Vec;

unsafe extern "C" {
    fn open_notes(db_path: *const c_char) -> *mut c_void;
    fn close_notes(db_handle: *mut c_void) -> bool;
    fn add_note(db_handle: *mut c_void, note_text: *const c_char) -> bool;
    fn list_notes(db_handle: *mut c_void) -> *mut c_void;
    fn delete_note(db_handle: *mut c_void, note_id: c_int) -> bool;
    fn validate_schema(db_handle: *mut c_void) -> bool;

    fn free_result(result: *mut c_void);
}

#[derive(Debug)]
struct Note {
    values: Vec<String>,
    names: Vec<String>,
}

#[repr(C)]
struct QueryRowRaw {
    col_values: *const *const c_char,
    col_names: *const *const c_char,
    num_cols: usize,
}

#[repr(C)]
struct QueryResultRaw {
    rows: *const *const QueryRowRaw,
    num_rows: usize,
    capacity: usize,
}

struct QueryResultGuard {
    handle: *mut c_void,
}

impl QueryResultGuard {
    fn create_result(handle: *mut c_void) -> Result<QueryResultGuard, &'static str> {
        if handle.is_null() {
            return Err("Can't get results if they are null");
        }
        Ok(QueryResultGuard { handle: handle })
    }
    fn get(&self) -> Vec<Note> {
        let mut notes: Vec<Note> = Vec::new();
        let query_result = self.handle as *mut QueryResultRaw;
        let num_rows = unsafe { (*query_result).num_rows };
        if num_rows == 0 {
            return notes;
        }

        let rows = unsafe { std::slice::from_raw_parts((*query_result).rows, num_rows) };
        for row in rows {
            let num_cols = unsafe{ (*(*row)).num_cols };
            let col_names = unsafe { std::slice::from_raw_parts((*(*row)).col_names, num_cols) };
            let col_values = unsafe { std::slice::from_raw_parts((*(*row)).col_values, num_cols) };
            
            let mut note = Note { values: Vec::new(), names: Vec::new() };
            for i in 0..num_cols {
                let name = unsafe { CStr::from_ptr(col_names[i]) }.to_string_lossy().into_owned();
                let value = unsafe { CStr::from_ptr(col_values[i]) }.to_string_lossy().into_owned();
                note.values.push(value);
                note.names.push(name);
            }
            notes.push(note);
        }
        notes
    }
}

impl Drop for QueryResultGuard {
    fn drop(&mut self) {
        unsafe {
            free_result(self.handle);
        }
    }
}

struct Notes {
    db_handle: *mut c_void,
}

impl Notes {
    fn open(db_path: &str) -> Result<Notes, &'static str>  {
        let path = CString::new(db_path).expect("This shouldn't be called!");
        unsafe {
            let db_handle = open_notes(path.as_ptr());
            if db_handle.is_null() {
                return Err("Failed to get database handle");
            }
            if !validate_schema(db_handle) {
                close_notes(db_handle);
                return Err("Failed to validate database schema");
            }
            Ok(Notes { db_handle: db_handle })
        }
    }

    fn add(&self, note_text_view: &str) -> bool {
        let note_text = CString::new(note_text_view).expect("Shouldn't need this");
        unsafe {
            add_note(self.db_handle, note_text.as_ptr())
        }
    }

    fn list(&self) -> Vec<Note> {
        let query_result_handle = unsafe { list_notes(self.db_handle) };
        let query_result = QueryResultGuard::create_result(query_result_handle).expect("This is not expected");
        query_result.get()
    }

    fn delete(&self, note_id: i32) -> bool {
        unsafe {
            delete_note(self.db_handle, note_id)
        }
    }
}

impl Drop for Notes {
    fn drop(&mut self) {
        let closed_status = unsafe {
            close_notes(self.db_handle)
        };
        if !closed_status {
            eprintln!("Failed to close DB properly!");
        }
    }
}

fn main()
{
    let notes = Notes::open("../db.sqlite").expect("Something");
    // notes.add("This is yet another note from rust using a struct");
    let notes = notes.list();
    for note in notes {
        println!("{0:?} {1:?}", note.names, note.values)
    }
}
