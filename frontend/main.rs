mod ffi;

use ffi::Notes;

fn main() -> Result<(), &'static str>
{
    let notes_db = Notes::open("../db.sqlite")?;
    let notes = notes_db.list()?;
    for note in notes {
        println!("{0:?} {1:?}", note.names, note.values)
    }
    Ok(())
}
