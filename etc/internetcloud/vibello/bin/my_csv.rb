# CSV extension that lets you quote some fields
# by the Tin Man, Feb 6 2011
# http://stackoverflow.com/questions/4854900/how-do-i-force-one-field-in-rubys-csv-output-to-be-wrapped-with-double-quotes
require 'csv'

class MyCSV < CSV
    def <<(row)
      # make sure headers have been assigned
      if header_row? and [Array, String].include? @use_headers.class
        parse_headers  # won't read data for Array or String
        self << @headers if @write_headers
      end

      # handle CSV::Row objects and Hashes
      row = case row
        when self.class::Row then row.fields
        when Hash            then @headers.map { |header| row[header] }
        else                      row
      end

      @headers = row if header_row?
      @lineno  += 1

      @do_quote ||= lambda do |field|
        field         = String(field)
        encoded_quote = @quote_char.encode(field.encoding)
        encoded_quote                                +
        field.gsub(encoded_quote, encoded_quote * 2) +
        encoded_quote
      end

      @quotable_chars      ||= encode_str("\r\n", @col_sep, @quote_char)
      @forced_quote_fields ||= []

      @my_quote_lambda ||= lambda do |field, index|
        if field.nil?  # represent +nil+ fields as empty unquoted fields
          ""
        else
          field = String(field)  # Stringify fields
          # represent empty fields as empty quoted fields
          if (
            field.empty?                          or
            field.count(@quotable_chars).nonzero? or
            @forced_quote_fields.include?(index)
          )
            @do_quote.call(field)
          else
            field  # unquoted field
          end
        end
      end

      output = row.map.with_index(&@my_quote_lambda).join(@col_sep) + @row_sep  # quote and separate
      if (
        @io.is_a?(StringIO)             and
        output.encoding != raw_encoding and
        (compatible_encoding = Encoding.compatible?(@io.string, output))
      )
        @io = StringIO.new(@io.string.force_encoding(compatible_encoding))
        @io.seek(0, IO::SEEK_END)
      end
      @io << output

      self  # for chaining
    end
    alias_method :add_row, :<<
    alias_method :puts,    :<<

    def open_writer(path, mode, fs, rs, &block)
      file = File.open(path, mode)
      if block
        begin
          MyCSV::Writer.generate(file, fs, rs) do |writer|
            yield(writer)
          end
        ensure
          file.close
        end
        nil
      else
        writer = MyCSV::Writer.create(file, fs, rs) 
        writer.close_on_terminate
        writer
      end
    end

    def forced_quote_fields=(indexes=[])
      @forced_quote_fields = indexes
    end
end

