# encoding: utf-8

$:.unshift File.dirname(__FILE__)

require 'tasks_db'

header = <<EOF
\\documentclass[a4paper,11pt]{report}

\\usepackage[cm]{fullpage}

\\usepackage[english,russian]{babel}
\\usepackage[utf8]{inputenc}
\\usepackage[T2A]{fontenc}
\\usepackage{indentfirst}
\\usepackage{cmap}
\\usepackage{tabularx}
\\usepackage{array}

\\begin{document}
EOF

footer = <<EOF
\\end{document}
EOF

curs = ['Основы искусственного интеллекта', 'Проектирование программ в интеллектуальных системах']

paths = [
  '\\\\Info\\StudInfo\\~Методическое обеспечение кафедры\\~Учебные курсы\\1 курс\\ОИИ\\Расчётная работа',
  '\\\\Info\\StudInfo\\~Методическое обеспечение кафедры\\~Учебные курсы\\2 курс\\ППвИС\\1sem\\Расчётная работа'
]

def end_group(to, last_group)
  to.puts '  \hline'
  to.puts '  \end{tabularx}'
  to.puts '\end{table}'
  to.puts
  to.puts '\clearpage' unless last_group
end

if $PROGRAM_NAME == __FILE__
  if ARGV.size != 1
    puts "Error: Pass zero-based semester number."
    exit(1)
  end

  sem = ARGV[0].to_i
  db = TasksDb.new 'tasks.csv'

  File.open 'students_tasks.tex', 'w' do |to|
    to.puts header

    first_group = true
    counter = 0

    CSV.foreach 'students_tasks.csv', col_sep: ';' do |row|
      if row[1]
        counter = counter + 1

        to << "  {\\tiny #{counter}} & {\\tiny #{row[0]}} &"

        if row[1] == '~'
          to << '{\tiny ~}'
        else
          tgr_num = row[1].to_i - 1
          t_num = row[2].to_i - 1
          
          to << '{\tiny '
          to << "#{db.groups[tgr_num][0]}: " if tgr_num == 0 or tgr_num == 1
          to << "#{db.groups[tgr_num][1][t_num].name}"
          
          specs = row[3..row.size - 1].reject(&:nil?)
          to << ' (' << specs.join(', ') << ')' unless specs.empty?
          
          to << '}'
        end

        to.puts ' & ~ & ~ \\\\ \hline'
        to.puts
      else
        if first_group
          first_group = false
        else
          end_group to, false
        end

        to.puts '\begin{center}'
        to << '  ' << "\\textbf{Группа #{row[0].force_encoding("UTF-8")}}" << "\n"
        to.puts '\end{center}'

        to.puts
        to.puts "Материалы для выполния расчетной работы можно найти по следующему сетевому пути:"
        to.puts '{\scriptsize'
        to.puts '\begin{verbatim}'
        to << paths[sem] << "\n"
        to.puts '\end{verbatim}'
        to.puts '}'

        to.puts

        to.puts "Задание расчетной работы по курсу <<#{curs[sem]}>> получил (-a):"

        to.puts

        to.puts '\begin{table}[ht]'
        to.puts '  \centering'
        to.puts '  \begin{tabularx}{\linewidth}{| c | c | X | c | c |}'
        to.puts '  \hline'
        to << '  {\scriptsize №} & {\scriptsize ФИО студента} & {\scriptsize Задание по расчетной работе}' <<
            '  & {\scriptsize Дата получения} & {\scriptsize Подпись} \\\\ \hline' << "\n"
        counter = 0
      end
    end

    end_group to, true

    to.puts footer
  end
end
