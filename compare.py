import re

def parse_log(filename):
    data = {}
    with open(filename, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        i = 0
        while i < len(lines):
            if lines[i].startswith('$'):
                question_id = int(lines[i].strip('$').strip())
                metric = lines[i + 1].split(': ')[1].strip()
                time = float(lines[i + 2].split(': ')[1].strip())
                data[question_id] = (int(metric), time)
                i += 3
            else:
                i += 1
    return data

def compare_logs(log1, log2):
    total_points_1, total_time_1 = 0, 0.0
    total_points_2, total_time_2 = 0, 0.0
    point_wins, point_losses, point_draws = 0, 0, 0

    for qid in log1.keys():
        points_1, time_1 = log1[qid]
        points_2, time_2 = log2[qid]
        total_points_1 += points_1
        total_time_1 += time_1
        total_points_2 += points_2
        total_time_2 += time_2

        if points_1 > points_2:
            point_wins += 1
        elif points_1 < points_2:
            point_losses += 1
        else:
            point_draws += 1

    return {
        'log1_total_points': total_points_1,
        'log1_total_time': total_time_1,
        'log2_total_points': total_points_2,
        'log2_total_time': total_time_2,
        'point_wins': point_wins,
        'point_losses': point_losses,
        'point_draws': point_draws
    }

if __name__ == "__main__":
    log1_data = parse_log("log1.txt")
    log2_data = parse_log("log2.txt")
    stats = compare_logs(log1_data, log2_data)
    
    print("Log1 總填點數:", stats['log1_total_points'])
    print("Log1 總時間:", stats['log1_total_time'])
    print("Log2 總填點數:", stats['log2_total_points'])
    print("Log2 總時間:", stats['log2_total_time'])
    print("Log1 贏的題數:", stats['point_wins'])
    print("Log2 贏的題數:", stats['point_losses'])
    print("平手的題數:", stats['point_draws'])
