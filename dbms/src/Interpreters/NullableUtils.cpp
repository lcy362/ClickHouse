#include <Interpreters/NullableUtils.h>


namespace DB
{

void extractNestedColumnsAndNullMap(ColumnRawPtrs & key_columns, ColumnPtr & null_map_holder, ConstNullMapPtr & null_map)
{
    if (key_columns.size() == 1)
    {
        auto & column = key_columns[0];
        if (!column->isColumnNullable())
            return;

        const ColumnNullable & column_nullable = static_cast<const ColumnNullable &>(*column);
        null_map = &column_nullable.getNullMapData();
        column = &column_nullable.getNestedColumn();
    }
    else
    {
        PaddedPODArray<UInt8> * mutable_null_map = nullptr;

        for (auto & column : key_columns)
        {
            if (column->isColumnNullable())
            {
                const ColumnNullable & column_nullable = static_cast<const ColumnNullable &>(*column);
                column = &column_nullable.getNestedColumn();

                if (!null_map_holder)
                {
                    null_map_holder = column_nullable.getNullMapColumnPtr()->clone();
                    mutable_null_map = &static_cast<ColumnUInt8 &>(*null_map_holder).getData();
                }
                else
                {
                    mutable_null_map = &static_cast<ColumnUInt8 &>(*null_map_holder).getData();
                    const PaddedPODArray<UInt8> & other_null_map = column_nullable.getNullMapData();
                    for (size_t i = 0, size = mutable_null_map->size(); i < size; ++i)
                        (*mutable_null_map)[i] |= other_null_map[i];
                }
            }
        }

        null_map = mutable_null_map;
    }
}

}
