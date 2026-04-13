import sys

BOARD_SIZE = 25


def parse_steps(path):
    """Parse opt_step.txt style file into list of boards.

    Each board block format:
    $<step>
    <25 lines of 0/1/? with tabs>
    """
    boards = []
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f]

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line.startswith("$"):
            # Skip step id, read next 25 lines as a board
            i += 1
            grid = []
            for _ in range(BOARD_SIZE):
                if i >= len(lines):
                    break
                row = lines[i].strip()
                # Split by tab if present, otherwise treat as continuous chars
                if "\t" in row:
                    cells = row.split("\t")
                else:
                    cells = list(row)
                # Normalize to characters '0','1','?' only
                cells = [c.strip() for c in cells if c.strip()]
                grid.append(cells)
                i += 1
            if len(grid) == BOARD_SIZE:
                boards.append(grid)
        else:
            i += 1
    return boards


def diff_boards(b1, b2):
    """Return number of different cells and list of (r,c,v1,v2)."""
    diffs = []
    for r in range(BOARD_SIZE):
        for c in range(BOARD_SIZE):
            v1 = b1[r][c] if c < len(b1[r]) else "?"
            v2 = b2[r][c] if c < len(b2[r]) else "?"
            if v1 != v2:
                diffs.append((r, c, v1, v2))
    return len(diffs), diffs


def main():
    if len(sys.argv) not in (3, 4):
        print("用法: python board_diff.py <未優化檔> <優化檔> [輸出檔]")
        print("例如: python board_diff.py old_opt_step.txt opt_step.txt diff_result.txt")
        sys.exit(1)

    path1, path2 = sys.argv[1], sys.argv[2]
    out_path = sys.argv[3] if len(sys.argv) == 4 else "diff_result.txt"

    boards1 = parse_steps(path1)
    boards2 = parse_steps(path2)

    with open(out_path, "w", encoding="utf-8") as out:
        n1, n2 = len(boards1), len(boards2)
        out.write(f"{path1} 步數: {n1}\n")
        out.write(f"{path2} 步數: {n2}\n")

        first_diff_step = None
        first_missing = []  # (r,c,before,after) where before=1, after!=1

        max_common = min(n1, n2)
        for idx in range(max_common):
            cnt, diffs = diff_boards(boards1[idx], boards2[idx])
            if cnt == 0:
                out.write(f"第 {idx+1} 步: 盤面完全相同\n")
            else:
                out.write(f"第 {idx+1} 步: {cnt} 個格子不同\n")
                # 只印前幾個差異避免太長
                for r, c, v1, v2 in diffs[:10]:
                    out.write(f"  ({r},{c}): {path1}={v1}, {path2}={v2}\n")
                if cnt > 10:
                    out.write("  ... 其餘差異省略\n")

                if first_diff_step is None:
                    first_diff_step = idx + 1
                    # 找出這一步中「未優化版是 1、優化版不是 1」的格子
                    for r, c, v1, v2 in diffs:
                        if v1 == "1" and v2 != "1":
                            first_missing.append((r, c, v1, v2))

        if n1 != n2:
            out.write("步數不同，後面多出來的步數無法一一對應比較\n")

        # 總結：最早分歧步驟與少掉的填點
        out.write("\n==== 總結 ====""\n")
        if first_diff_step is None:
            out.write("兩個版本在共同步數內盤面完全相同。\n")
        else:
            out.write(f"最早分歧發生在第 {first_diff_step} 步。\n")
            if first_missing:
                out.write("在這一步中，優化前為 1、優化後不是 1 的格子:\n")
                for r, c, _v1, v2 in first_missing:
                    out.write(f"  ({r},{c}): before=1, after={v2}\n")
            else:
                out.write("這一步沒有出現『優化前是 1、優化後不是 1』的格子。\n")

    print(f"比較結果已輸出到 {out_path}")


if __name__ == "__main__":
    main()
