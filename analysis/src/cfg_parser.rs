#[derive(Debug)]
pub struct Cfg {
    pub blocks: Vec<Block>,
}

#[derive(Debug)]
pub struct Block {
    pub id: usize,
    pub kind: BlockKind,
}

#[derive(Debug)]
pub enum BlockKind {
    Goto(usize),
    End,
}

use std::fs;

pub fn parse_cfg(path: &str) -> Cfg {
    let content = fs::read_to_string(path).expect("failed to read file");

    let mut blocks = Vec::new();
    let mut current_id: Option<usize> = None;

    for line in content.lines() {
        let line = line.trim();

        if line.is_empty() {
            continue;
        }

        
        if line.starts_with("block") {
            let parts: Vec<&str> = line.split_whitespace().collect();
            let id_str = parts[1].trim_end_matches(':');
            let id = id_str.parse::<usize>().unwrap();

            current_id = Some(id);
        }
        
        else if line.starts_with("goto") {
            let target = line
                .split_whitespace()
                .nth(1)
                .unwrap()
                .parse::<usize>()
                .unwrap();

            blocks.push(Block {
                id: current_id.expect("block not started"),
                kind: BlockKind::Goto(target),
            });
        }
        
        else if line == "end" {
            blocks.push(Block {
                id: current_id.expect("block not started"),
                kind: BlockKind::End,
            });
        }
    }

    Cfg { blocks }
}