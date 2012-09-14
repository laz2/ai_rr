
$:.unshift File.dirname(__FILE__)

require 'tasks_db'

if $PROGRAM_NAME == __FILE__
  db = TasksDb.new 'tasks.csv'

  File.open 'tasks_list.tex', 'w' do |f|
    f.puts
    f.puts '\begin{enumerate}'

    db.groups.each do |g|
      f.puts "\\item #{g[0]}:"

      f.puts '  \begin{enumerate}[1.]'

      g[1].each do |t|
        f.puts "  \\item #{t.to_str}"
      end

      f.puts '  \end{enumerate}'
    end

    f.puts '\end{enumerate}'

    f.puts

    f.puts <<EOF
%%% Local Variables:
%%% mode: latex
%%% TeX-master: "ai_rr"
%%% End:
EOF
  end
end

