
$:.unshift File.dirname(__FILE__)

require 'tasks_db'

header = <<EOF
\documentclass[a4paper,11pt]{report}

\usepackage[cm]{fullpage}

\usepackage[english,russian]{babel}
\usepackage[utf8]{inputenc}
\usepackage[T2A]{fontenc}
\usepackage{indentfirst}
\usepackage{cmap}

\begin{document}
EOF

footer = <<EOF
\end{document}
EOF

curs = ['Основы искусственного интеллекта', 'Проектирование программ в интеллектуальных системах']
paths = [
  '\\Info\StudInfo\~Методическое обеспечение кафедры\~Учебные курсы\1 курс\ОИИ\Расчётная работа',
  '\\Info\StudInfo\~Методическое обеспечение кафедры\~Учебные курсы\2 курс\ППвИС\1sem\Расчётная работа'
]

if $PROGRAM_NAME == __FILE__
  db = TasksDb.new 'tasks.csv'

  File.open 'students_tasks.tex', 'w' do |to|
    to.puts header

    File.open 'students_tasks.in' do |from|

    end

    to.puts footer
  end
end
