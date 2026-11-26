mod ffi;

use ffi::Notes;

fn main()
{
    let notes = Notes::open("../db.sqlite").expect("Something");
    // notes.add("This is yet another note from rust using a struct");
    let notes = notes.list();
    for note in notes {
        println!("{0:?} {1:?}", note.names, note.values)
    }
}
