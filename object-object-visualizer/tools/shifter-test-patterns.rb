Pattern = Struct.new(:probId, :rotates, :splits, :axis)

pattern = [
  Pattern.new(5, [1, 3], [3, 4], :X),
  Pattern.new(8, [1], [4], :X),
  Pattern.new(7, [1, 3], [4, 5], :X),
  Pattern.new(9, [0], [3, 4], :X),
  Pattern.new(12, [0, 2], [4, 5], :X),
  Pattern.new(15, [0, 2], [4], :X),
  Pattern.new(18, [0, 1, 2], [3, 4, 5], :X),
  Pattern.new(20, [1, 2, 3], [3, 4, 5], :X),
  Pattern.new(22, [0, 2], [5, 6], :X),
  Pattern.new(24, [0, 1, 2, 3], [4, 5], :X)
]

patterns2 = ((1..25).to_a + [38, 39]).map { |probId|
  [
    Pattern.new(probId, (0..3).to_a, (3..8).to_a, :X),
    Pattern.new(probId, (0..3).to_a, (3..8).to_a, :Y),
  ]
}.flatten

rules = []
patterns2.each do |pat|
  pat.rotates.each do |r|
    pat.splits.each do |s|
      pat_name = "rot#{r}-sp#{s}-#{pat.axis}"
      rule = "prob#{pat.probId}-#{pat_name}"
      command_name = if pat.probId == 38 || pat.probId == 39
                       '../../kawatea/dp2'
                     else
                       '../../kawatea/dp'
                     end

      puts "#{rule}:"
      puts "\tnpm run meta -- --problemId #{pat.probId} --batchName shift-test-#{pat_name} --command '#{command_name}' --rotate #{r} --split #{s} --splitAxis #{pat.axis}"
      puts ""
      rules << rule
    end
  end
end

puts "all: #{rules.join(' ')}"