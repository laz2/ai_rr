
require 'CSV'
require 'stringio'

class TasksDb
  attr_reader :groups

  class Task
    attr_reader :complexity
    attr_reader :name
    attr_reader :types

    def initialize(row)
      if row[0] == '?'
        @complexity = -1
      else
        @complexity = row[0].to_i
      end

      @name = row[1]

      @types = []
      row[2..(row.size - 1)].each do |t|
        @types.push t if t
      end
    end

    def to_str
      s = StringIO.new

      s << '('
      if @complexity == -1
        s << '?'
      else
        s << @complexity
      end
      s << ') '

      s << @name << ' '

      unless @types.empty?
        s << '('
        s << @types.join(', ')
        s << ')'
      end

      s.string
    end
  end

  def initialize(path = nil)
    @groups = []
    read_from path if path
  end

  def read_from(path)
    CSV.foreach(path, col_sep: ';') do |row|
      if row[1]
        @groups.last[1].push Task.new(row)
      else
        @groups.push [row[0], []]
      end
    end
  end

  def to_str
    s = StringIO.new

    @groups.each_with_index do |g, i|
      s << i + 1 << '. ' << g[0] << ':' << "\n"
      g[1].each_with_index do |t, j|
        s << '  ' << j + 1 << '. ' << t.to_str << "\n"
      end
    end

    s.string
  end
end

