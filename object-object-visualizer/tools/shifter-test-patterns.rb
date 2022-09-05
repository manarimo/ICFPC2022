Pattern = Struct.new(:probId, :rotates, :splits)

pattern = [
  Pattern.new(5, [1, 3], [3, 4]),
  Pattern.new(8, [1], [4]),
  Pattern.new(7, [1, 3], [4, 5]),
  Pattern.new(9, [0], [3, 4]),
  Pattern.new(12, [0, 2], [4, 5]),
  Pattern.new(15, [0, 2], [4]),
  Pattern.new(20, [1, 2, 3], [3, 4, 5]),
  Pattern.new(24, [0, 1, 2, 3], [4, 5])
]

rules = []
pattern.each do |pat|
  pat.rotates.each do |r|
    pat.splits.each do |s|
      rule = "prob#{pat.probId}-rot#{r}-sp#{s}"
      puts "#{rule}:"
      puts "\tnpm run meta -- --problemId #{pat.probId} --batchName shift-test-$@ --command '../../kawatea/a.out' --rotate #{r} --split #{s}"
      puts ""
      rules << rule
    end
  end
end

puts "all: #{rules.join(' ')}"