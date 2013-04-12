// Copyright Steinwurf ApS 2011-2013.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <cstdint>

#include <fifi/fifi_utils.hpp>

namespace kodo
{

    /// @ingroup debug
    /// @brief Debug layer which allows inspecting the state of a
    ///        linear block decoder.
    template<class SuperCoder>
    class debug_linear_block_decoder : public SuperCoder
    {
    public:

        /// @copydoc layer::field_type
        typedef typename SuperCoder::field_type field_type;

        /// @copydoc layer::value_type
        typedef typename field_type::value_type value_type;

        /// @copydoc layer::factory factory
        typedef typename SuperCoder::factory factory;

        /// Access to status about uncoded symbols in the linear block
        /// decoder.
        using SuperCoder::m_uncoded;

        /// Access to status about the coded symbols in the linear block
        /// decoder.
        using SuperCoder::m_coded;

    public:

        /// @copydoc layer::construct(factory&)
        void construct(factory& the_factory)
        {
            SuperCoder::construct(the_factory);

            m_data.resize(the_factory.max_symbol_size());
            m_coefficients.resize(the_factory.max_coefficients_size());
        }

        /// Prints the decoding matrix to the output stream
        /// @param out The output stream to print to
        void print_decoding_matrix(std::ostream &out)
        {
            for(uint32_t i = 0; i < SuperCoder::symbols(); ++i)
            {
                if( m_uncoded[i] )
                {
                    out << i << " U:\t";
                }
                else if( m_coded[i] )
                {
                    out << i << " C:\t";
                }
                else
                {
                    out << i << " ?:\t";
                }

                value_type *coefficients_i =
                    SuperCoder::coefficients_value(i);

                for(uint32_t j = 0; j < SuperCoder::symbols(); ++j)
                {

                    value_type value =
                        fifi::get_value<field_type>(coefficients_i, j );

                    out << (uint32_t)value << " ";
                }

                out << std::endl;
            }

            out << std::endl;
        }

        /// @copydoc layer::decode_symbol(uint8_t*,uint8_t*)
        void decode_symbol(uint8_t *symbol_data,
                           uint8_t *symbol_coefficients)
        {
            assert(symbol_data != 0);
            assert(symbol_coefficients != 0);

            uint32_t symbol_size = SuperCoder::symbol_size();
            uint32_t coef_size = SuperCoder::coefficients_size();

            auto data_dest = sak::storage(m_data);
            auto coef_dest = sak::storage(m_coefficients);

            auto data_src = sak::storage(symbol_data, symbol_size);
            auto coef_src = sak::storage(symbol_coefficients, coef_size);

            sak::copy_storage(data_dest, data_src);
            sak::copy_storage(coef_dest, coef_src);

            m_symbol_coded = true;

            SuperCoder::decode_symbol(symbol_data, symbol_coefficients);
        }

        /// @copydoc layer::decode_symbol(uint8_t*,uint32_t)
        void decode_symbol(const uint8_t *symbol_data,
                           uint32_t symbol_index)
        {
            assert(symbol_data != 0);
            assert(symbol_index < SuperCoder::symbols());

            uint32_t symbol_size = SuperCoder::symbol_size();

            auto data_dest = sak::storage(m_data);
            auto data_src = sak::storage(symbol_data, symbol_size);

            sak::copy_storage(data_dest, data_src);

            m_symbol_index = symbol_index;
            m_symbol_coded =  false;

            SuperCoder::decode_symbol(symbol_data, symbol_index);
        }

        /// Prints the most recent symbol coefficients to enter the decoder
        /// @param out The output stream to print to
        void print_decoding_symbol(std::ostream &out)
        {
            if( m_symbol_coded )
            {
                out << "Symbol C:" << std::endl<< "\t Coef.: ";

                value_type *c =
                    reinterpret_cast<value_type*>(&m_coefficients[0]);

                for(uint32_t j = 0; j < SuperCoder::symbols(); ++j)
                {
                    value_type value = fifi::get_value<field_type>(c, j);
                    out << (uint32_t)value << " ";
                }

                out << std::endl;
            }
            else
            {
                out << "Symbol U:" << std::endl;
                out << "\tIndex: " << m_symbol_index << std::endl;
            }
        }

    private:

        /// Tracks whether the recent decoding symbol was coded
        bool m_symbol_coded;

        /// If the symbol what uncoded what was the symbol index
        uint32_t m_symbol_index;

        /// Stores the data of the decoding symbol
        std::vector<uint8_t> m_data;

        /// If coded stores the coefficients of the decoding symbol
        std::vector<uint8_t> m_coefficients;

    };

}

#endif

